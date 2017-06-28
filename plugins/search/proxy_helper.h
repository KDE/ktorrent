/***************************************************************************
 *   Copyright (C) 2017 by Alexander Trufanov                              *
 *   trufanovan@gmail.com                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#ifndef KT_PROXY_HELPER_H
#define KT_PROXY_HELPER_H

#include <QUrl>

#include <KIO/MetaData>
#include <dbus/dbussettings.h>

#include "searchpluginsettings.h"

namespace kt
{

    class ProxyHelper
    {
        DBusSettings* m_settings;
    public:
        ProxyHelper(DBusSettings* settings);
        void setSettings(DBusSettings* settings) {m_settings = settings;}
        bool ApplyProxy (KIO::MetaData& metadata) const;
    };

}

#endif // KT_HOMEPAGE_H
