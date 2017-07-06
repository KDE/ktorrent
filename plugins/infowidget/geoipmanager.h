/***************************************************************************
 *   Copyright (C) 2009 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
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
        GeoIPManager(QObject* parent = 0);
        ~GeoIPManager();

        /**
         * Find the country given an IP address
         * @param addr The IP address
         * @return The country ID
         */
        int findCountry(const QString& addr);

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
        static QUrl geoIPUrl() {return geoip_url;}

        /// Set the database URL
        static void setGeoIPUrl(const QUrl &url);

        /// Download the database
        void downloadDataBase();

    private slots:
        void databaseDownloadFinished(KJob* job);
        void decompressFinished();

    private:
        GeoIP* geo_ip;
        QString geoip_data_file;
        QString download_destination;
        bt::DecompressThread* decompress_thread;
        static QUrl geoip_url;
    };


}

#endif // KT_GEOIPMANAGER_H
