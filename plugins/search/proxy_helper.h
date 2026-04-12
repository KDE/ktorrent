/*
    SPDX-FileCopyrightText: 2017 Alexander Trufanov <trufanovan@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KT_PROXY_HELPER_H
#define KT_PROXY_HELPER_H

#include <QNetworkProxy>
#include <QUrl>

#include <KIO/MetaData>
#include <settings.h>

#include "searchpluginsettings.h"

class QNetworkAccessManager;

namespace kt
{
class ProxyHelper
{
public:
    bool ApplyProxy(KIO::MetaData &metadata) const;
    void applyNetworkProxy(QNetworkAccessManager *manager) const;
    QNetworkProxy networkProxy() const;
};
}

#endif // KT_HOMEPAGE_H
