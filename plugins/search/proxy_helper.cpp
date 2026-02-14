/*
    SPDX-FileCopyrightText: 2017 Alexander Trufanov <trufanovan@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "proxy_helper.h"

namespace kt
{
bool ProxyHelper::ApplyProxy(KIO::MetaData &metadata) const
{
    if (!SearchPluginSettings::openInExternal() && SearchPluginSettings::useProxySettings()) {
        if (!Settings::useKDEProxySettings() && !Settings::httpProxy().trimmed().isEmpty()) {
            QString p = QStringLiteral("%1:%2").arg(Settings::httpProxy()).arg(Settings::httpProxyPort());
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
