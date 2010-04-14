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
#include <download/webseed.h>
#include <torrent/choker.h>
#include <peer/authenticationmonitor.h>
#include <peer/peermanager.h>
#include <peer/utpex.h>
#include <net/socketmonitor.h>
#include <net/socks.h>
#include <dht/dhtbase.h>
#include <mse/streamsocket.h>
#include <tracker/httptracker.h>
#include <tracker/udptrackersocket.h>
#include <diskio/chunkmanager.h>
#include <diskio/cache.h>
#include <torrent/torrentcontrol.h>
#include <util/log.h>
#include <torrent/server.h>
#include <torrent/timeestimator.h>
#include <interfaces/queuemanagerinterface.h>
#include "settings.h"
#include "functions.h"

using namespace bt;

namespace kt
{

	
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
		mse::StreamSocket::setTOS(Settings::dscp() << 2);
		mse::StreamSocket::setMaxConnecting(Settings::maxConnectingSockets());
		ChunkManager::setUploadDataCheckingEnabled(Settings::doUploadDataCheck());
		if (!Settings::useMaxSizeForUploadDataCheck())
			ChunkManager::setMaxChunkSizeForDataCheck(0);
		else
			ChunkManager::setMaxChunkSizeForDataCheck(Settings::maxSizeForUploadDataCheck() * 1024);
	
		UDPTrackerSocket::setPort(Settings::udpTrackerPort());
		Choker::setNumUploadSlots(Settings::numUploadSlots());

		dht::DHTBase & ht = Globals::instance().getDHT();
		if (Settings::dhtSupport() && !ht.isRunning())
		{
			ht.start(kt::DataDir() + "dht_table",kt::DataDir() + "dht_key",Settings::dhtPort());
		}
		else if (!Settings::dhtSupport() && ht.isRunning())
		{
			ht.stop();
		}
		else if (Settings::dhtSupport() && ht.getPort() != Settings::dhtPort())
		{
			Out(SYS_GEN|LOG_NOTICE) << "Restarting DHT with new port " << Settings::dhtPort() << endl;
			ht.stop();
			ht.start(kt::DataDir() + "dht_table",kt::DataDir() + "dht_key",Settings::dhtPort());
		}
		
		UTPex::setEnabled(Settings::pexEnabled());
		
		if (Settings::useEncryption())
		{
			ServerInterface::enableEncryption(Settings::allowUnencryptedConnections());
		}
		else
		{
			ServerInterface::disableEncryption();
		}
	
		if (Settings::useCustomIP())
			Tracker::setCustomIP(Settings::customIP());
		else
			Tracker::setCustomIP(QString::null);
		
		
		QString proxy = Settings::httpProxy();
	;
		bt::HTTPTracker::setProxyEnabled(!Settings::useKDEProxySettings() && Settings::useProxyForTracker());
		bt::HTTPTracker::setProxy(proxy,Settings::httpProxyPort());
		bt::HTTPTracker::setUseQHttp(Settings::useQHttpAnnounce());
		bt::WebSeed::setProxy(proxy,Settings::httpProxyPort());
		bt::WebSeed::setProxyEnabled(!Settings::useKDEProxySettings() && Settings::useProxyForWebSeeds());
		bt::Cache::setPreallocationEnabled(Settings::diskPrealloc());
		bt::Cache::setPreallocateFully(Settings::fullDiskPrealloc());
		
		bt::TorrentControl::setDataCheckWhenCompleted(Settings::checkWhenFinished());
		bt::TorrentControl::setMinimumDiskSpace(Settings::minDiskSpace());
		bt::TorrentControl::setAutoRecheck(Settings::autoRecheck());
		bt::TorrentControl::setNumCorruptedForRecheck(Settings::maxCorruptedBeforeRecheck());
		
		
		if (Settings::networkInterface() == 0)
		{
			SetNetworkInterface(QString::null);
		}
		else
		{
			QList<QNetworkInterface> iface_list = QNetworkInterface::allInterfaces();
			int iface = Settings::networkInterface();
			if (iface > iface_list.count())
				SetNetworkInterface(QString::null);
			else
				SetNetworkInterface(iface_list[iface - 1].name());
		}
		
		net::Socks::setSocksEnabled(Settings::socksEnabled());
		net::Socks::setSocksVersion(Settings::socksVersion());
		net::Socks::setSocksServerAddress(Settings::socksProxy(),Settings::socksPort());
		if (Settings::socksUsePassword())
			net::Socks::setSocksAuthentication(Settings::socksUsername(),Settings::socksPassword());
		else
			net::Socks::setSocksAuthentication(QString::null,QString::null);
		
		bt::TimeEstimator::setAlgorithm((bt::TimeEstimator::ETAlgorithm)Settings::eta());
		
		bt::ChunkManager::setPreviewSizes(Settings::previewSizeAudio() * 1024,Settings::previewSizeVideo() * 1024);
		bt::QueueManagerInterface::setQueueManagerEnabled(!Settings::manuallyControlTorrents());
		bt::Downloader::setUseWebSeeds(Settings::webseedsEnabled());
		bt::Peer::setResolveHostnames(Settings::lookUpHostnameOfPeers());
	}


}
