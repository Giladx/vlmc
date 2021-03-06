/*****************************************************************************
 * Effect.h: Handle a frei0r effect.
 *****************************************************************************
 * Copyright (C) 2008-2010 VideoLAN
 *
 * Authors: Hugo Beauzée-Luyssen <beauze.h@gmail.com>
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

#ifndef EFFECT_H
#define EFFECT_H

#include <QLibrary>

#include "frei0r.h"

class   EffectInstance;

class Effect : public QLibrary
{
    public:
        static const quint32        TrackEffectDefaultLength = 25 * 60 * 10; //10 minutes at 25fps
        enum    Type
        {
            Unknown = -1,
            Filter = F0R_PLUGIN_TYPE_FILTER,
            Source = F0R_PLUGIN_TYPE_SOURCE,
            Mixer2 = F0R_PLUGIN_TYPE_MIXER2,
            Mixer3 = F0R_PLUGIN_TYPE_MIXER3
        };
        struct  Parameter
        {
            char*   name;
            char*   desc;
            int     type;
            Parameter( const char *_name, const char* _desc, int _type ) :
                    name( strdup( _name ) ), desc( strdup( _desc ) ), type( _type ) {}
        };

        typedef     QList<Parameter*>       ParamList;

        typedef     int (*f0r_init_t)();
        typedef     void (*f0r_deinit_t)();
        typedef     void (*f0r_get_info_t)(f0r_plugin_info_t*);
        typedef     f0r_instance_t (*f0r_construct_t)( unsigned int, unsigned int );
        typedef     void (*f0r_destruct_t)( f0r_instance_t );
        typedef     void (*f0r_update_t)( f0r_instance_t, double, const unsigned int*, unsigned int * );
        typedef     void (*f0r_update2_t)( f0r_instance_t, double, const unsigned int*, const unsigned int*, const unsigned int*, unsigned int * );
        typedef     void (*f0r_get_param_value_t)( f0r_instance_t, f0r_param_t, int );
        typedef     void (*f0r_set_param_value_t)( f0r_instance_t, f0r_param_t, int );
        typedef     void (*f0r_get_param_info_t)( f0r_param_info_t*, int );

        Effect( const QString& fileName );
        virtual ~Effect();

        bool            load();
        const QString&  name();
        const QString&  description();
        Type            type();
        const QString&  author();
        const ParamList &params() const;
        //This breaks coding convention, but it would be safe just to undef major/minor.
        int             getMajor();
        int             getMinor();
        EffectInstance  *createInstance();

    private:
        void            destroyInstance( EffectInstance* instance );

    private:
        QString                     m_name;
        QString                     m_desc;
        Type                        m_type;
        int                         m_major;
        int                         m_minor;
        QString                     m_author;
        int                         m_nbParams;
        QAtomicInt                  m_instCount;
        ParamList                   m_params;

        //Symbols:
        f0r_init_t                  m_f0r_init;
        f0r_deinit_t                m_f0r_deinit;
        f0r_get_info_t              m_f0r_info;
        f0r_construct_t             m_f0r_construct;
        f0r_destruct_t              m_f0r_destruct;
        f0r_update_t                m_f0r_update;
        f0r_update2_t               m_f0r_update2;
        f0r_get_param_value_t       m_f0r_get_param_value;
        f0r_set_param_value_t       m_f0r_set_param_value;
        f0r_get_param_info_t        m_f0r_get_param_info;

        friend class    EffectInstance;
        friend class    FilterInstance;
        friend class    MixerInstance;
        friend class    EffectSettingValue;
};

#endif // EFFECT_H
