/*****************************************************************************
 * TrackHandler.cpp : Handle multiple track of a kind (audio or video)
 *****************************************************************************
 * Copyright (C) 2008-2009 the VLMC team
 *
 * Authors: Hugo Beauzee-Luyssen <hugo@vlmc.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#include "LightVideoFrame.h"
#include "TrackHandler.h"
#include "TrackWorkflow.h"

#include <QDomDocument>
#include <QDomElement>

LightVideoFrame* TrackHandler::nullOutput = NULL;

TrackHandler::TrackHandler( unsigned int nbTracks, MainWorkflow::TrackType trackType,
                            EffectsEngine* effectsEngine ) :
        m_trackCount( nbTracks ),
        m_trackType( trackType ),
        m_length( 0 ),
        m_effectEngine( effectsEngine )
{
    TrackHandler::nullOutput = new LightVideoFrame();

    m_tracks = new Toggleable<TrackWorkflow*>[nbTracks];
    for ( unsigned int i = 0; i < nbTracks; ++i )
    {
        m_tracks[i].setPtr( new TrackWorkflow( i, trackType ) );
        connect( m_tracks[i], SIGNAL( trackEndReached( unsigned int ) ), this, SLOT( trackEndReached(unsigned int) ) );
    }
}

TrackHandler::~TrackHandler()
{
    if ( nullOutput != NULL )
    {
        delete nullOutput;
        nullOutput = NULL;
    }
    for (unsigned int i = 0; i < m_trackCount; ++i)
        delete m_tracks[i];
    delete[] m_tracks;
}

void
TrackHandler::addClip( Clip* clip, unsigned int trackId, qint64 start )
{
    Q_ASSERT_X( trackId < m_trackCount, "MainWorkflow::addClip",
                "The specified trackId isn't valid, for it's higher than the number of tracks");

    m_tracks[trackId]->addClip( clip, start );
    //if the track is deactivated, we need to reactivate it.
    if ( m_tracks[trackId].deactivated() == true )
        activateTrack( trackId );

    //Now check if this clip addition has changed something about the workflow's length
    if ( m_tracks[trackId]->getLength() > m_length )
        m_length = m_tracks[trackId]->getLength();
}

void
TrackHandler::startRender()
{
    m_paused = false;
    m_endReached = false;
    for ( unsigned int i = 0; i < m_trackCount; ++i )
        activateTrack( i );
    computeLength();
}

void
TrackHandler::computeLength()
{
    qint64      maxLength = 0;

    for ( unsigned int i = 0; i < m_trackCount; ++i )
    {
        if ( m_tracks[i]->getLength() > maxLength )
            maxLength = m_tracks[i]->getLength();
    }
    m_length = maxLength;
}

qint64
TrackHandler::getLength() const
{
    return m_length;
}

void
TrackHandler::getOutput( qint64 currentFrame )
{
    m_tmpAudioBuffer = NULL;
    for ( unsigned int i = 0; i < m_trackCount; ++i )
    {
        if ( m_trackType == MainWorkflow::VideoTrack )
        {
            if ( m_tracks[i].activated() == false )
                m_effectEngine->setVideoInput( i + 1, *TrackHandler::nullOutput );
            else
            {
                void*   ret = m_tracks[i]->getOutput( currentFrame );
                if ( ret == NULL )
                    m_effectEngine->setVideoInput( i + 1, *TrackHandler::nullOutput );
                else
                {
                    StackedBuffer<LightVideoFrame*>* stackedBuffer =
                        reinterpret_cast<StackedBuffer<LightVideoFrame*>*>( ret );
                    m_effectEngine->setVideoInput( i + 1, *( stackedBuffer->get() ) );
                }
            }
        }
        else
        {
            void*   ret = m_tracks[i]->getOutput( currentFrame );
            //m_tmpAudioBuffer is NULl by default, so it will remain NULL if we continue;
            if ( ret == NULL )
                continue ;
            StackedBuffer<AudioClipWorkflow::AudioSample*>* stackedBuffer =
                reinterpret_cast<StackedBuffer<AudioClipWorkflow::AudioSample*>*> ( ret );
            if ( stackedBuffer != NULL )
                m_tmpAudioBuffer = stackedBuffer->get();
        }
    }
}

void
TrackHandler::pause()
{
    for ( unsigned int i = 0; i < m_trackCount; ++i )
    {
        if ( m_tracks[i].activated() == true )
            m_tracks[i]->pause();
    }
}

void
TrackHandler::unpause()
{
    for ( unsigned int i = 0; i < m_trackCount; ++i )
    {
        //Don't check for track activation, as it could have change from the time we paused.
        m_tracks[i]->unpause();
    }
}

void
TrackHandler::activateAll()
{
    for ( unsigned int i = 0; i < m_trackCount; ++i )
        activateTrack( i );
}

void
TrackHandler::activateTrack( unsigned int trackId )
{
    if ( m_tracks[trackId]->getLength() > 0 )
        m_tracks[trackId].activate();
    else
        m_tracks[trackId].deactivate();
}

qint64
TrackHandler::getClipPosition( const QUuid &uuid, unsigned int trackId ) const
{
    Q_ASSERT( trackId < m_trackCount );

    return m_tracks[trackId]->getClipPosition( uuid );
}

void
TrackHandler::stop()
{
    for (unsigned int i = 0; i < m_trackCount; ++i)
    {
        if ( m_tracks[i].activated() == true )
            m_tracks[i]->stop();
    }
}

void
TrackHandler::moveClip(const QUuid &clipUuid, unsigned int oldTrack,
                                       unsigned int newTrack, qint64 startingFrame )
{
     Q_ASSERT( newTrack < m_trackCount && oldTrack < m_trackCount );

    if ( oldTrack == newTrack )
    {
        //And now, just move the clip.
        m_tracks[newTrack]->moveClip( clipUuid, startingFrame );
        activateTrack( newTrack );
    }
    else
    {
        bool    needRepo;

        if ( m_tracks[oldTrack]->getClipPosition( clipUuid ) != startingFrame )
            needRepo = true;
        ClipWorkflow* cw = m_tracks[oldTrack]->removeClipWorkflow( clipUuid );
        m_tracks[newTrack]->addClip( cw, startingFrame );
        if ( needRepo == true )
            m_tracks[newTrack]->forceRepositionning();
        activateTrack( oldTrack );
        activateTrack( newTrack );
    }
    computeLength();
}

Clip*
TrackHandler::removeClip( const QUuid& uuid, unsigned int trackId )
{
    Q_ASSERT( trackId < m_trackCount );

    Clip* clip = m_tracks[trackId]->removeClip( uuid );
    computeLength();
    activateTrack( trackId );
    return clip;
}

void
TrackHandler::muteTrack( unsigned int trackId )
{
    m_tracks[trackId].setHardDeactivation( true );
}

void
TrackHandler::unmuteTrack( unsigned int trackId )
{
    m_tracks[trackId].setHardDeactivation( false );
}

Clip*
TrackHandler::getClip( const QUuid& uuid, unsigned int trackId )
{
    Q_ASSERT( trackId < m_trackCount );

    return m_tracks[trackId]->getClip( uuid );
}

void
TrackHandler::clear()
{
    for ( unsigned int i = 0; i < m_trackCount; ++i )
    {
        m_tracks[i]->clear();
    }
    m_length = 0;
}

bool
TrackHandler::isPaused() const
{
    return m_paused;
}

bool
TrackHandler::endIsReached() const
{
    return m_endReached;
}

void
TrackHandler::trackEndReached( unsigned int trackId )
{
    m_tracks[trackId].deactivate();

    for ( unsigned int i = 0; i < m_trackCount; ++i)
    {
        if ( m_tracks[i].activated() == true )
            return ;
    }
    m_endReached = true;
    emit tracksEndReached();
}

unsigned int
TrackHandler::getTrackCount() const
{
    return m_trackCount;
}

void
TrackHandler::save( QDomDocument& doc, QDomElement& timelineNode ) const
{
    for ( unsigned int i = 0; i < m_trackCount; ++i)
    {
        if ( m_tracks[i]->getLength() > 0 )
        {
            QDomElement     trackNode = doc.createElement( "track" );

            trackNode.setAttribute( "id", i );

            m_tracks[i]->save( doc, trackNode );
            timelineNode.appendChild( trackNode );
        }
    }
}
