/*****************************************************************************
 * ClipRenderer.cpp: Render from a Clip (mainly for previewing purpose)
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Geoffroy Lacarrière <geoffroylaca@gmail.com>
 *          Hugo Beauzee-Luyssen <hugo@vlmc.org>
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

#include <QtDebug>
#include <QtGlobal>
#include <QtCore/qmath.h>

#include "Clip.h"
#include "ClipRenderer.h"
#include "Library.h"
#include "Media.h"
#include "MainWorkflow.h"
#include "VLCMediaPlayer.h"
#include "VLCMedia.h"

ClipRenderer::ClipRenderer() :
    GenericRenderer(),
    m_clipLoaded( false ),
    m_vlcMedia( NULL ),
    m_selectedClip( NULL ),
    m_begin( 0 ),
    m_end( -1 ),
    m_mediaChanged( false )
{
    connect( m_mediaPlayer,     SIGNAL( stopped() ),            this,   SLOT( __videoStopped() ) );
    connect( m_mediaPlayer,     SIGNAL( paused() ),             this,   SIGNAL( paused() ) );
    connect( m_mediaPlayer,     SIGNAL( playing() ),            this,   SIGNAL( playing() ) );
    connect( m_mediaPlayer,     SIGNAL( volumeChanged() ),      this,   SIGNAL( volumeChanged() ) );
    connect( m_mediaPlayer,     SIGNAL( timeChanged( qint64 ) ),this,   SLOT( __timeChanged( qint64 ) ) );
    connect( m_mediaPlayer,     SIGNAL( endReached() ),         this,   SLOT( __endReached() ) );
}

ClipRenderer::~ClipRenderer()
{
    if ( m_vlcMedia )
        delete m_vlcMedia;
    stop();
}

void
ClipRenderer::setClip( Clip* clip )
{
    // if the clip is different (or NULL) we have to stop playback.
    if ( m_selectedClip != NULL &&
         ( ( clip != NULL && clip->uuid() != m_selectedClip->uuid() ) || clip == NULL ) )
    {
        clipUnloaded( m_selectedClip->uuid() );
    }
    if ( clip == NULL )
    {
        m_selectedClip = NULL;
        m_clipLoaded = false;
        return ;
    }
    m_selectedClip = clip;
    if ( clip->length() == 0 )
        return ;
    updateInfos( clip );
}

void
ClipRenderer::updateInfos( Clip* clip )
{
    m_begin = clip->begin();
    m_end = clip->end();
    if ( m_isRendering == true )
        m_mediaChanged = true;
    else
        m_clipLoaded = false;
}

void
ClipRenderer::startPreview()
{
    if ( m_selectedClip == NULL || m_selectedClip->length() == 0 )
        return ;
    updateInfos( m_selectedClip );

    //If an old media is found, we delete it, and disconnect
    if ( m_vlcMedia != NULL )
        delete m_vlcMedia;
    m_vlcMedia = new LibVLCpp::Media( m_selectedClip->getMedia()->mrl() );

    m_mediaPlayer->setKeyInput( false );
    m_mediaPlayer->setMedia( m_vlcMedia );

    m_mediaPlayer->play();
    m_mediaPlayer->setPosition( (double)m_begin / (double)m_selectedClip->getMedia()->nbFrames() );
    m_clipLoaded = true;
    m_isRendering = true;
    m_paused = false;
    m_mediaChanged = false;
}

void
ClipRenderer::stop()
{
    if ( m_clipLoaded == true && m_isRendering == true )
    {
        m_isRendering = false;
        m_mediaPlayer->stop();
        m_paused = false;
        if ( m_mediaChanged == true )
            m_clipLoaded = false;
    }
}

void
ClipRenderer::togglePlayPause( bool forcePause )
{
    if ( m_clipLoaded == false )
    {
        emit frameChanged( 0, Vlmc::Renderer );
        startPreview();
        return ;
    }
    if ( m_paused == false && m_isRendering == true )
    {
        m_mediaPlayer->pause();
        m_paused = true;
    }
    else if ( forcePause == false )
    {
        if ( m_isRendering == false )
        {
            m_mediaPlayer->play();
            m_mediaPlayer->setPosition( m_begin / ( m_end - m_begin ) );
            m_isRendering = true;
        }
        else
            m_mediaPlayer->play();
        m_paused = false;
    }
}

int
ClipRenderer::getVolume() const
{
    return m_mediaPlayer->getVolume();
}

int
ClipRenderer::setVolume( int volume )
{
    //Returns 0 if the volume was set, -1 if it was out of range
    return m_mediaPlayer->setVolume( volume );
}

void
ClipRenderer::nextFrame()
{
    if ( m_isRendering == true )
    {
        m_mediaPlayer->nextFrame();
    }
}

void
ClipRenderer::previousFrame()
{
    if ( m_isRendering == true )
    {
        if ( m_paused == false )
            togglePlayPause( true );
        /* FIXME: Implement a better way to render previous frame */
        qint64   interval =  static_cast<qint64>( qCeil(1000.0f * 2.0f / m_mediaPlayer->getFps()) );
        m_mediaPlayer->setTime( m_mediaPlayer->getTime() - interval );
        m_mediaPlayer->nextFrame();
    }
}

qint64
ClipRenderer::length() const
{
    return m_end - m_begin;
}

qint64
ClipRenderer::getLengthMs() const
{
    if ( m_selectedClip )
        return ( qRound64( (qreal)( m_end - m_begin ) / m_selectedClip->getMedia()->fps() * 1000.0 ) );
    return 0;
}

void
ClipRenderer::clipUnloaded( const QUuid& uuid )
{
    if ( m_selectedClip != NULL && m_selectedClip->uuid() == uuid )
    {
        m_mediaPlayer->stop();
        m_clipLoaded = false;
        m_selectedClip = NULL;
        m_isRendering = false;
        m_paused = false;
    }
}

qint64
ClipRenderer::getCurrentFrame() const
{
    if ( m_clipLoaded == false || m_isRendering == false || m_selectedClip == NULL )
        return 0;
    return qRound64( (qreal)m_mediaPlayer->getTime() / 1000 *
                     (qreal)m_selectedClip->getMedia()->fps() );
}

float
ClipRenderer::getFps() const
{
    if ( m_selectedClip != NULL )
        return m_selectedClip->getMedia()->fps();
    return 0.0f;
}

Clip*
ClipRenderer::getClip()
{
    return m_selectedClip;
}

void
ClipRenderer::previewWidgetCursorChanged( qint64 newFrame )
{
    if ( m_isRendering == true )
    {
        newFrame += m_begin;
        qint64 nbSeconds = qRound64( (qreal)newFrame / m_selectedClip->getMedia()->fps() );
        m_mediaPlayer->setTime( nbSeconds * 1000 );
    }
}

/////////////////////////////////////////////////////////////////////
/////SLOTS :
/////////////////////////////////////////////////////////////////////

void
ClipRenderer::__videoStopped()
{
    emit frameChanged( 0, Vlmc::Renderer );
    emit stopped();
}

void
ClipRenderer::__timeChanged( qint64 time )
{
    float   fps = (qreal)m_mediaPlayer->getFps();
    if ( fps < 0.1f )
        fps = m_selectedClip->getMedia()->fps();
    qint64 f = qRound64( (qreal)time / 1000.0 * fps );
    if ( f >= m_end )
    {
        __endReached();
        return ;
    }
    f = f - m_begin;
    emit frameChanged( f, Vlmc::Renderer );
}

void
ClipRenderer::__endReached()
{
    m_mediaPlayer->stop();
    m_isRendering = false;
    if ( m_mediaChanged == true )
        m_clipLoaded = false;
    emit endReached();
}
