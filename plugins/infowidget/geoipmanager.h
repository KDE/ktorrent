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
#include <KUrl>

#ifdef USE_SYSTEM_GEOIP
#include <GeoIP.h>
#else
#include "GeoIP.h"
#endif

class KJob;

namespace kt 
{
	class DecompressThread;
	
	/**
	 * Manages GeoIP database. Downloads it from the internet and handles all queries to it.
	 */
	class GeoIPManager : public QObject
	{
		Q_OBJECT
	public:
		GeoIPManager(QObject* parent = 0);
		virtual ~GeoIPManager();
		
		/**
		 * Find the country given an IP address
		 * @param addr The IP address
		 * @return The country ID
		 */
		int findCountry(const QString & addr);
		
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
		static KUrl geoIPUrl() {return geoip_url;}
		
		/// Set the database URL
		static void setGeoIPUrl(const KUrl & url);
		
		/// Download the database
		void downloadDataBase();
		
	private slots:
		void databaseDownloadFinished(KJob* job);
		void decompressFinished();
	
	private:
		GeoIP* geo_ip;
		QString geoip_data_file;
		QString download_destination;
		DecompressThread* decompress_thread;
		static KUrl geoip_url;
	};
	
	/**
	 * Thread which decompresses a single file
	 */
	class DecompressThread : public QThread
	{
	public:
		DecompressThread(const QString & file,const QString & dest_file);
		virtual ~DecompressThread();
		
		/// Run the decompression thread
		virtual void run();
		
		/// Cancel the thread, things should be cleaned up properly
		void cancel();
		
		/// Get the error which happened (0 means no error)
		int error() const {return err;}
		
	private:
		QString file;
		QString dest_file;
		bool canceled;
		int err;
	};
}

#endif // KT_GEOIPMANAGER_H
