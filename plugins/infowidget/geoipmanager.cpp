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
#include <QFile>
#include <kstandarddirs.h>
#include <kio/job.h>
#include <kio/copyjob.h>
#include <kfilterdev.h>
#include <util/log.h>
#include <interfaces/functions.h>
#include <util/fileops.h>
#include "geoipmanager.h"
#include <QFileInfo>
#include <QDateTime>

using namespace bt;

namespace kt
{
	KUrl GeoIPManager::geoip_url = KUrl("http://geolite.maxmind.com/download/geoip/database/GeoLiteCountry/GeoIP.dat.gz");
	
	GeoIPManager::GeoIPManager(QObject* parent): QObject(parent),geo_ip(0),decompress_thread(0)
	{
#ifdef USE_SYSTEM_GEOIP
		geo_ip = GeoIP_open_type(GEOIP_COUNTRY_EDITION, GEOIP_STANDARD);
#else
		geoip_data_file = KStandardDirs::locate("data", "ktorrent/geoip.dat");
		if (geoip_data_file.isNull())
			geoip_data_file = KStandardDirs::locate("data", "ktorrent/GeoIP.dat");
		
		if (geoip_data_file.isNull())
		{
			downloadDataBase();
		}
		else
		{
			geo_ip = GeoIP_open(QFile::encodeName(geoip_data_file),0);
			if (geo_ip)
			{
				QFileInfo fi(geoip_data_file);
				QDateTime now = QDateTime::currentDateTime();
				if (fi.lastModified().daysTo(now) >= 30)
				{
					// if the last time the geoip file was modified, was more then
					// 30 days ago, redownload it
					downloadDataBase();
				}
			}
		}
#endif
	}

	GeoIPManager::~GeoIPManager()
	{
		if (geo_ip)
			GeoIP_delete(geo_ip);
		
		if (decompress_thread)
		{
			decompress_thread->cancel();
			decompress_thread->wait();
			delete decompress_thread;
		}
	}

	int GeoIPManager::findCountry(const QString& addr)
	{
		if (!geo_ip)
			return 0;
		else
			return GeoIP_id_by_name(geo_ip, addr.toAscii());
	}

	QString GeoIPManager::countryCode(int country_id)
	{
		if (country_id > 0)
			return GeoIP_country_code[country_id];
		else 
			return QString();
	}

	QString GeoIPManager::countryName(int country_id)
	{
		if (country_id > 0)
			return GeoIP_country_name[country_id];
		else
			return QString();
	}
	
	void GeoIPManager::downloadDataBase()
	{
#ifndef USE_SYSTEM_GEOIP
		Out(SYS_INW|LOG_NOTICE) << "Downloading GeoIP database: " << geoip_url.prettyUrl() << endl;
		download_destination = kt::DataDir() + geoip_url.fileName();
		KIO::CopyJob* job = KIO::copy(geoip_url,download_destination,KIO::Overwrite|KIO::HideProgressInfo);
		connect(job,SIGNAL(result(KJob*)),this,SLOT(databaseDownloadFinished(KJob*)));
#endif
	}
	
	void GeoIPManager::databaseDownloadFinished(KJob* job)
	{
		if (job->error())
		{
			Out(SYS_INW|LOG_IMPORTANT) << "Failed to download GeoIP database: " << job->errorString() << endl;
			return;
		}
		
		if (download_destination.endsWith(".dat") || download_destination.endsWith(".DAT"))
		{
			Out(SYS_INW|LOG_NOTICE) << "GeoIP database downloaded, opening ...  " << endl;
			geoip_data_file = download_destination;
			if (geo_ip)
			{
				GeoIP_delete(geo_ip);
				geo_ip = 0;
			}
			geo_ip = GeoIP_open(QFile::encodeName(geoip_data_file),0);
			if (!geo_ip)
				Out(SYS_INW|LOG_NOTICE) << "Failed to open GeoIP database  " << endl;
		}
		else 
		{
			Out(SYS_INW|LOG_NOTICE) << "GeoIP database downloaded, decompressing ...  " << endl;
			// decompress the file
			decompress_thread = new DecompressThread(download_destination,kt::DataDir() + "geoip.dat");
			connect(decompress_thread,SIGNAL(finished()),this,SLOT(decompressFinished()),Qt::QueuedConnection);
			decompress_thread->start(QThread::IdlePriority);
		}
	}

	void GeoIPManager::decompressFinished()
	{
		Out(SYS_INW|LOG_NOTICE) << "GeoIP database decompressed, opening ...  " << endl;
		if (!decompress_thread->error())
		{
			geoip_data_file = kt::DataDir() + "geoip.dat";
			if (geo_ip)
			{
				GeoIP_delete(geo_ip);
				geo_ip = 0;
			}
			geo_ip = GeoIP_open(QFile::encodeName(geoip_data_file),0);
			if (!geo_ip)
				Out(SYS_INW|LOG_NOTICE) << "Failed to open GeoIP database  " << endl;
		}
		
		decompress_thread->wait();
		delete decompress_thread;
		decompress_thread = 0;
	}


	void GeoIPManager::setGeoIPUrl(const KUrl& url)
	{
		geoip_url = url;
	}
	
	///////////////////////////////////
	
	DecompressThread::DecompressThread(const QString & file,const QString & dest_file) 
		: file(file),dest_file(dest_file),canceled(false),err(0)
	{
	}
	
	DecompressThread::~DecompressThread()
	{}
	
	void DecompressThread::run()
	{
		QFile out(dest_file);
		
		// open input file readonly
		if (!out.open(QIODevice::WriteOnly))
		{
			err = KIO::ERR_CANNOT_OPEN_FOR_WRITING;
			Out(SYS_INW|LOG_NOTICE) << "Failed to open " << dest_file << " : " << out.errorString() << endl;
			return;
		}
		
		// open output file 
		QIODevice* dev = KFilterDev::deviceForFile(file);
		if (!dev || !dev->open(QIODevice::ReadOnly))
		{
			err = KIO::ERR_CANNOT_OPEN_FOR_READING;
			if (dev)
				Out(SYS_INW|LOG_NOTICE) << "Failed to open " << file << " : " << dev->errorString() << endl;
			else
				Out(SYS_INW|LOG_NOTICE) << "Failed to open " << file << endl;
			return;
		}
		
		// copy the data
		char buf[4096];
		while (!canceled && !dev->atEnd())
		{
			int len = dev->read(buf,4096);
			if (len == 0)
				break;
			
			out.write(buf,len);
		}
		
		delete dev;
		out.close();
		if (canceled)
		{
			// delete output file when canceled
			bt::Delete(dest_file,true);
		}
		else
		{
			// delete the input file upon success
			bt::Delete(file,true);
		}
	}
	
	void DecompressThread::cancel()
	{
		canceled = true;
	}


}

