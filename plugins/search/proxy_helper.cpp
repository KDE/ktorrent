/*
    SPDX-FileCopyrightText: 2017 Alexander Trufanov <trufanovan@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "proxy_helper.h"

#include <QNetworkAccessManager>

namespace kt
{
bool ProxyHelper::ApplyProxy(KIO::MetaData &metadata) const
{
    if (!SearchPluginSettings::openInExternal() && SearchPluginSettings::useProxySettings()) {
        if (!Settings::useKDEProxySettings() && !Settings::httpProxy().trimmed().isEmpty()) {
            QString p = QStringLiteral("%1:%2").arg(Settings::httpProxy()).arg(Settings::httpProxyPort());
            if (!p.startsWith(QLatin1String("http://"))) {
                p = QStringLiteral("http://") + p;
            }

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

QNetworkProxy ProxyHelper::networkProxy() const
{
    if (!SearchPluginSettings::openInExternal() && SearchPluginSettings::useProxySettings() && !Settings::useKDEProxySettings()
        && !Settings::httpProxy().trimmed().isEmpty()) {
        QString proxy = Settings::httpProxy().trimmed();
        if (!proxy.startsWith(QLatin1String("http://")) && !proxy.startsWith(QLatin1String("https://"))) {
            proxy = QStringLiteral("http://") + proxy;
        }

        const QUrl proxyUrl(proxy);
        if (proxyUrl.isValid() && !proxyUrl.host().isEmpty()) {
            return QNetworkProxy(QNetworkProxy::HttpProxy, proxyUrl.host(), proxyUrl.port(Settings::httpProxyPort()));
        }
    }

    return QNetworkProxy(QNetworkProxy::DefaultProxy);
}

void ProxyHelper::applyNetworkProxy(QNetworkAccessManager *manager) const
{
    if (manager) {
        manager->setProxy(networkProxy());
    }
}
}
