/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
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
#include <qdatetime.h>
#include <QNetworkInterface>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <solid/device.h>
#include <solid/networkinterface.h>
#include <util/functions.h>
#include <download/downloader.h>
#include <torrent/choker.h>
#include <peer/authenticationmonitor.h>
#include <peer/peermanager.h>
#include <net/socketmonitor.h>
#include <dht/dhtbase.h>
#include <mse/streamsocket.h>
#include <tracker/tracker.h>
#include <tracker/udptrackersocket.h>
#include <diskio/chunkmanager.h>
#include <util/log.h>
#include <torrent/server.h>
#include "settings.h"
#include "functions.h"

using namespace bt;

namespace kt
{
	const double TO_KB = 1024.0;
	const double TO_MEG = (1024.0 * 1024.0);
	const double TO_GIG = (1024.0 * 1024.0 * 1024.0);
	
	QString BytesToString(Uint64 bytes,int precision)
	{
		KLocale* loc = KGlobal::locale();
		if (bytes >= 1024 * 1024 * 1024)
			return i18n("%1 GB",loc->formatNumber(bytes / TO_GIG,precision < 0 ? 2 : precision));
		else if (bytes >= 1024*1024)
			return i18n("%1 MB",loc->formatNumber(bytes / TO_MEG,precision < 0 ? 1 : precision));
		else if (bytes >= 1024)
			return i18n("%1 KB",loc->formatNumber(bytes / TO_KB,precision < 0 ? 1 : precision));
		else
			return i18n("%1 B",bytes);
	}

	QString KBytesPerSecToString(double speed,int precision)
	{
		KLocale* loc = KGlobal::locale();
		return i18n("%1 KB/s",loc->formatNumber(speed,precision));
	}

	QString DurationToString(Uint32 nsecs)
	{
		KLocale* loc = KGlobal::locale();
		QTime t;
		int ndays = nsecs / 86400;
		t = t.addSecs(nsecs % 86400);
		QString s = loc->formatTime(t,true,true);
		if (ndays > 0)
			s = i18np("1 day ","%1 days ",ndays) + s;

		return s;
	}
	
	QString DataDir()
	{
		QString str = KGlobal::dirs()->saveLocation("data","ktorrent");
		if (!str.endsWith(bt::DirSeparator()))
			return str + bt::DirSeparator();
		else
			return str;
	}
	
	void ApplySettings()
	{
		PeerManager::setMaxConnections(Settings::maxConnections());
		PeerManager::setMaxTotalConnections(Settings::maxTotalConnections());
		net::SocketMonitor::setDownloadCap(Settings::maxDownloadRate()*1024);
		net::SocketMonitor::setUploadCap(Settings::maxUploadRate()*1024);
		net::SocketMonitor::setSleepTime(Settings::cpuUsage());
		mse::StreamSocket::setTOS(Settings::typeOfService());
		ChunkManager::setUploadDataCheckingEnabled(Settings::doUploadDataCheck());
		if (!Settings::useMaxSizeForUploadDataCheck())
			ChunkManager::setMaxChunkSizeForDataCheck(0);
		else
			ChunkManager::setMaxChunkSizeForDataCheck(Settings::maxSizeForUploadDataCheck() * 1024);
	
		UDPTrackerSocket::setPort(Settings::udpTrackerPort());
		Downloader::setMemoryUsage(Settings::memoryUsage());
		Choker::setNumUploadSlots(Settings::numUploadSlots());

		dht::DHTBase & ht = Globals::instance().getDHT();
		if (Settings::dhtSupport() && !ht.isRunning())
		{
			ht.start(kt::DataDir() + "dht_table",Settings::dhtPort());
		}
		else if (!Settings::dhtSupport() && ht.isRunning())
		{
			ht.stop();
		}
		else if (Settings::dhtSupport() && ht.getPort() != Settings::dhtPort())
		{
			Out(SYS_GEN|LOG_NOTICE) << "Restarting DHT with new port " << Settings::dhtPort() << endl;
			ht.stop();
			ht.start(kt::DataDir() + "dht_table",Settings::dhtPort());
		}
		
		if (Settings::useEncryption())
		{
			Globals::instance().getServer().enableEncryption(Settings::allowUnencryptedConnections());
		}
		else
		{
			Globals::instance().getServer().disableEncryption();
		}
	
		if (Settings::useExternalIP())
			Tracker::setCustomIP(Settings::externalIP());
		else
			Tracker::setCustomIP(QString::null);
	}

	QString NetworkInterface()
	{
		if (Settings::networkInterface() == 0)
			return QString::null;

		QList<QNetworkInterface> iface_list = QNetworkInterface::allInterfaces();
		int iface = Settings::networkInterface();
		if (iface > iface_list.count())
			return QString::null;

		return iface_list[iface - 1].name();
	}
	
	QString NetworkInterfaceIPAddress(const QString & iface)
	{
		QNetworkInterface ni = QNetworkInterface::interfaceFromName(iface);
		if (!ni.isValid())
			return QString::null;

		QList<QNetworkAddressEntry> addr_list = ni.addressEntries();
		if (addr_list.count() == 0)
			return QString::null;
		else
			return addr_list.front().ip().toString();
	}

}
