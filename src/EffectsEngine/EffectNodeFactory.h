/*****************************************************************************
 * EffectNodeFactory.h: this class is used to instantiate a new EffectNode
 *                      which contains builtin or plugin effect
 *****************************************************************************
 * Copyright (C) 2008-2009 the VLMC team
 *
 * Authors: Vincent Carrubba <cyberbouba@gmail.com>
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

#ifndef EFFECTNODEFACTORY_H_
#define EFFECTNODEFACTORY_H_

#include <QMap>

#include "GreenFilterEffectPluginCreator.h"
#include "MixerEffectPluginCreator.h"

#include "IEffectPluginCreator.h"
#include "EffectNode.h"

class	EffectNodeFactory
{
 public:

  // CTOR & DTOR

    EffectNodeFactory();
    ~EffectNodeFactory();

  EffectNode*        getEffect( quint32 id );

 private:

  QMap<QByteArray, IEffectPluginCreator*>       m_epc;

};

#endif // EFFECTNODEFACTORY_H_
