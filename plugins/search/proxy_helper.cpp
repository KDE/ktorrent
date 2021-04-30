/*
    SPDX-FileCopyrightText: 2017 Alexander Trufanov <trufanovan@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "proxy_helper.h"

namespace kt
{
ProxyHelper::ProxyHelper(DBusSettings *settings)
    : m_settings(settings)
{
}

bool ProxyHelper::ApplyProxy(KIO::MetaData &metadata) const
{
    if (!SearchPluginSettings::openInExternal() && SearchPluginSettings::useProxySettings() && m_settings) {
        if (!m_settings->useKDEProxySettings() && !m_settings->httpProxy().trimmed().isEmpty()) {
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
