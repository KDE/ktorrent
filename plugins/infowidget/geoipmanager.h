/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KT_GEOIPMANAGER_H
#define KT_GEOIPMANAGER_H

#include <QObject>
#include <GeoIP.h>

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

private:
    GeoIP *geo_ip;
};

}

#endif // KT_GEOIPMANAGER_H
