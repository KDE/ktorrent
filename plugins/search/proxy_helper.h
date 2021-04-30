/*
    SPDX-FileCopyrightText: 2017 Alexander Trufanov <trufanovan@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
    DBusSettings *m_settings;

public:
    ProxyHelper(DBusSettings *settings);
    void setSettings(DBusSettings *settings)
    {
        m_settings = settings;
    }
    bool ApplyProxy(KIO::MetaData &metadata) const;
};

}

#endif // KT_HOMEPAGE_H
