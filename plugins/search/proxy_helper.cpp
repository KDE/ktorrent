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

#include "proxy_helper.h"

namespace kt
{

ProxyHelper::ProxyHelper(DBusSettings* settings): m_settings(settings)
{

}

bool ProxyHelper::ApplyProxy(KIO::MetaData& metadata) const
{
    if (!SearchPluginSettings::openInExternal() &&
            SearchPluginSettings::useProxySettings() &&
            m_settings)
    {
        if (!m_settings->useKDEProxySettings() &&
            !m_settings->httpProxy().trimmed().isEmpty())
        {
            QString p = QStringLiteral("%1:%2").arg(m_settings->httpProxy()).arg(m_settings->httpProxyPort());
            if (!p.startsWith(QLatin1String("http://")))
                p = QStringLiteral("http://") + p;

            if (!QUrl(p).isValid()) {
                p = QString();
            }

            metadata[QStringLiteral("UseProxy")] = p;
            metadata[QStringLiteral("ProxyUrls")] = p;
        }

        return true;
    }

    return false;
}

}
