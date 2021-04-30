/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KT_GEOIPMANAGER_H
#define KT_GEOIPMANAGER_H

#include <QObject>
#include <QThread>
#include <QUrl>

#ifdef USE_SYSTEM_GEOIP
#include <GeoIP.h>
#else
#include "GeoIP.h"
#endif

class KJob;

namespace bt
{
class DecompressThread;
}

namespace kt
{
/**
 * Manages GeoIP database. Downloads it from the internet and handles all queries to it.
 */
class GeoIPManager : public QObject
{
    Q_OBJECT
public:
    GeoIPManager(QObject *parent = 0);
    ~GeoIPManager() override;

    /**
     * Find the country given an IP address
     * @param addr The IP address
     * @return The country ID
     */
    int findCountry(const QString &addr);

    /**
     * Get the name of the country
     * @param country_id The country ID
     * @return The name
     */
    QString countryName(int country_id);

    /**
     * Get the code of the country
     * @param country_id The country ID
     * @return The name
     */
    QString countryCode(int country_id);

    /// Get the database URL
    static QUrl geoIPUrl()
    {
        return geoip_url;
    }

    /// Set the database URL
    static void setGeoIPUrl(const QUrl &url);

    /// Download the database
    void downloadDataBase();

private Q_SLOTS:
    void databaseDownloadFinished(KJob *job);
    void decompressFinished();

private:
    GeoIP *geo_ip;
    QString geoip_data_file;
    QString download_destination;
    bt::DecompressThread *decompress_thread;
    static QUrl geoip_url;
};

}

#endif // KT_GEOIPMANAGER_H
