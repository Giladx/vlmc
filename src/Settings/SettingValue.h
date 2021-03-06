/*****************************************************************************
 * SettingValue.h: A setting value that can broadcast its changes
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

#ifndef SETTINGVALUE_H
#define SETTINGVALUE_H

#include <QObject>
#include <QVariant>

/**
 * 'class SettingValue
 *
 * \brief represent a setting value
 *
 */
class   SettingValue : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY( SettingValue );
    public:
        enum    Type
        {
            Int,
            Double,
            String,
            Bool,
            Language,
            KeyboardShortcut,
            Path,
            //For effect engine settings:
            Color,
            Position
        };
        enum    Flag
        {
            Nothing         = 0,
            /// If this flag is used, then the variable should not be shown in the config widgets.
            Private         = 1 << 0,
            Password        = 1 << 1,
            Clamped         = 1 << 2, ///< When used, the m_min and m_max will be used
            EightMultiple   = 1 << 3, ///< Forces the value to be a multiple of 8
            NotEmpty        = 1 << 4, ///< Forces the value not to be empty (likely to be user only with Strings)
        };
        Q_DECLARE_FLAGS( Flags, Flag );

        /**
         *  \brief      Constructs a setting value with its default value and description
         *
         *  \param      defaultValue    The setting default value.
         *  \param      desc            The setting description
         */
        SettingValue( Type type, const QVariant& defaultValue, const char* name,
                      const char* desc, Flags flags = Nothing );

        /**
         * \brief setter for the m_val member
         * \param   val the value wich will be affected to m_val
         */

        virtual void    set( const QVariant& val );

        /**
         * \brief getter for the m_val member
         */
        virtual const QVariant& get(); //Not const to avoid a mess with EffectSettingValue.
        /**
         *  \return The setting's description
         */
        const char      *description() const;
        /**
         *   \brief     Set the setting to its default value.
         */
        void            restoreDefault();

        const char      *name() const;
        Type            type() const;
        Flags           flags() const;

        void            setLimits( const QVariant& min, const QVariant& max );
        const QVariant& min() const;
        const QVariant& max() const;

    protected:
        /**
         * \brief the QVariant containingthe value of the settings
         */
        QVariant        m_val;
        QVariant        m_defaultVal;
        const char*     m_name;
        const char*     m_desc;
        Type            m_type;
        Flags           m_flags;
        QVariant        m_min;
        QVariant        m_max;
    signals:
        /**
         * \brief This signal is emmited while the m_val
         *        member have been modified
         */
        void        changed( const QVariant& );
};

#endif // SETTINGVALUE_H
