/*****************************************************************************
 * ImageClipWorkflow.h : Will extract a frame from an image
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
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

#ifndef IMAGECLIPWORKFLOW_H
#define IMAGECLIPWORKFLOW_H

#include "ClipWorkflow.h"

class   ImageClipWorkflow : public ClipWorkflow
{
    Q_OBJECT

    public:
        ImageClipWorkflow( ClipHelper* ch );
        ~ImageClipWorkflow();

        void                    *getLockCallback() const;
        void                    *getUnlockCallback() const;
        virtual Workflow::OutputBuffer  *getOutput( ClipWorkflow::GetMode mode, qint64 currentFrame );
        /**
         *  \brief      Deactivate time seeking in an ImageClipWorkflow
         */
        virtual void            setTime( qint64 ){}
    protected:
        virtual QString         createSoutChain() const;
        virtual void            initializeVlcOutput();
        virtual quint32         getNbComputedBuffers() const;
        virtual quint32         getMaxComputedBuffers() const;
        virtual void            flushComputedBuffers();
        virtual void            releasePrealocated(){}
    private:
        static void             lock( ImageClipWorkflow* clipWorkflow, void** pp_ret,
                                      int size );
        static void             unlock( ImageClipWorkflow* clipWorkflow, void* buffer,
                                        int width, int height, int bpp, int size,
                                        qint64 pts );
    private:
        Workflow::Frame             *m_buffer;
        EffectsEngine::EffectList   m_filters;
        Workflow::Frame             *m_effectFrame;

    private slots:
        void                    stopComputation();
    signals:
        void                    computedFinished();
};

#endif // IMAGECLIPWORKFLOW_H
