/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <KIO/CopyJob>

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>

#include "geoipmanager.h"
#include <interfaces/functions.h>
#include <util/decompressfilejob.h>
#include <util/fileops.h>
#include <util/log.h>

using namespace bt;

namespace kt
{
GeoIPManager::GeoIPManager(QObject *parent)
    : QObject(parent)
{
    geo_ip = GeoIP_open_type(GEOIP_COUNTRY_EDITION, GEOIP_STANDARD);
}

GeoIPManager::~GeoIPManager()
{
    if (geo_ip)
        GeoIP_delete(geo_ip);
}

int GeoIPManager::findCountry(const QString &addr)
{
    if (!geo_ip)
        return 0;
    else
        return GeoIP_id_by_name(geo_ip, addr.toLatin1().data());
}

QString GeoIPManager::countryCode(int country_id)
{
    if (country_id > 0 && country_id < 247)
        return QString::fromLatin1(GeoIP_country_code[country_id]);
    else
        return QString();
}

QString GeoIPManager::countryName(int country_id)
{
    if (country_id > 0 && country_id < 247)
        return QString::fromUtf8(GeoIP_country_name[country_id]);
    else
        return QString();
}

}
