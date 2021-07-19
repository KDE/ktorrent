/*
    SPDX-FileCopyrightText: 2005 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "functions.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QNetworkInterface>
#include <QRandomGenerator>
#include <QStandardPaths>

#include <KLocalizedString>
#include <Solid/Device>

#include "settings.h"
#include <dht/dhtbase.h>
#include <diskio/cache.h>
#include <diskio/chunkmanager.h>
#include <download/downloader.h>
#include <download/webseed.h>
#include <interfaces/queuemanagerinterface.h>
#include <mse/encryptedpacketsocket.h>
#include <net/socketmonitor.h>
#include <net/socks.h>
#include <peer/authenticationmonitor.h>
#include <peer/connectionlimit.h>
#include <peer/peerconnector.h>
#include <peer/peermanager.h>
#include <peer/utpex.h>
#include <torrent/choker.h>
#include <torrent/server.h>
#include <torrent/timeestimator.h>
#include <torrent/torrentcontrol.h>
#include <tracker/httptracker.h>
#include <tracker/udptrackersocket.h>
#include <util/functions.h>
#include <util/log.h>

using namespace bt;

namespace kt
{
QString DataDir(CreationMode mode)
{
    QString dataDirPath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    if (mode == CreateIfNotExists) {
        QFileInfo fileInfo(dataDirPath);
        if (!fileInfo.exists()) {
            QString ktorrent4DataFolder = QDir::homePath() + QLatin1String("/.kde/share/apps/ktorrent");
            if (!QFile::exists(ktorrent4DataFolder)) {
                ktorrent4DataFolder = QDir::homePath() + QLatin1String("/.kde4/share/apps/ktorrent");
                if (!QFile::exists(ktorrent4DataFolder))
                    ktorrent4DataFolder.clear();
            }
            if (ktorrent4DataFolder.isEmpty() || !QFile::rename(ktorrent4DataFolder, dataDirPath))
                fileInfo.dir().mkpath(fileInfo.fileName());
        }
    }
    // if (!str.endsWith(bt::DirSeparator()))
    return dataDirPath + bt::DirSeparator();
    // else
    //    return str;
}

Uint16 RandomGoodPort()
{
    Uint16 start = 50000;
    while (true) {
        Uint16 port = start + QRandomGenerator::global()->bounded(10000);
        if (port != Settings::port() && port != Settings::dhtPort() && port != Settings::udpTrackerPort())
            return port;
    }
}

void ApplySettings()
{
    PeerManager::connectionLimits().setLimits(Settings::maxTotalConnections(), Settings::maxConnections());
    net::SocketMonitor::setDownloadCap(Settings::maxDownloadRate() * 1024);
    net::SocketMonitor::setUploadCap(Settings::maxUploadRate() * 1024);
    net::SocketMonitor::setSleepTime(Settings::cpuUsage());
    mse::EncryptedPacketSocket::setTOS(Settings::dscp() << 2);
    bt::PeerConnector::setMaxActive(Settings::maxConnectingSockets());

    // Check for port conflicts
    if (Settings::port() == Settings::udpTrackerPort())
        Settings::setUdpTrackerPort(RandomGoodPort());

    if (Settings::port() == Settings::dhtPort())
        Settings::setDhtPort(RandomGoodPort());

    UDPTrackerSocket::setPort(Settings::udpTrackerPort());
    Choker::setNumUploadSlots(Settings::numUploadSlots());

    dht::DHTBase &ht = Globals::instance().getDHT();
    if (Settings::dhtSupport() && !ht.isRunning()) {
        ht.start(kt::DataDir() + QLatin1String("dht_table"), kt::DataDir() + QLatin1String("dht_key"), Settings::dhtPort());
    } else if (!Settings::dhtSupport() && ht.isRunning()) {
        ht.stop();
    } else if (Settings::dhtSupport() && ht.getPort() != Settings::dhtPort()) {
        Out(SYS_GEN | LOG_NOTICE) << "Restarting DHT with new port " << Settings::dhtPort() << endl;
        ht.stop();
        ht.start(kt::DataDir() + QLatin1String("dht_table"), kt::DataDir() + QLatin1String("dht_key"), Settings::dhtPort());
    }

    UTPex::setEnabled(Settings::pexEnabled());

    if (Settings::useEncryption()) {
        ServerInterface::enableEncryption(Settings::allowUnencryptedConnections());
    } else {
        ServerInterface::disableEncryption();
    }

    if (Settings::useCustomIP())
        Tracker::setCustomIP(Settings::customIP());
    else
        Tracker::setCustomIP(QString());

    QString proxy = Settings::httpProxy();

    bt::HTTPTracker::setProxyEnabled(!Settings::useKDEProxySettings() && Settings::useProxyForTracker());
    bt::HTTPTracker::setProxy(proxy, Settings::httpProxyPort());
    bt::WebSeed::setProxy(proxy, Settings::httpProxyPort());
    bt::WebSeed::setProxyEnabled(!Settings::useKDEProxySettings() && Settings::useProxyForWebSeeds());
    bt::Cache::setPreallocationEnabled(Settings::diskPrealloc());
    bt::Cache::setPreallocateFully(Settings::fullDiskPrealloc());

    bt::TorrentControl::setDataCheckWhenCompleted(Settings::checkWhenFinished());
    bt::TorrentControl::setMinimumDiskSpace(Settings::minDiskSpace());
    bt::SetNetworkInterface(Settings::networkInterface());
    net::Socks::setSocksEnabled(Settings::socksEnabled());
    net::Socks::setSocksVersion(Settings::socksVersion());
    net::Socks::setSocksServerAddress(Settings::socksProxy(), Settings::socksPort());
    if (Settings::socksUsePassword())
        net::Socks::setSocksAuthentication(Settings::socksUsername(), Settings::socksPassword());
    else
        net::Socks::setSocksAuthentication(QString(), QString());

    bt::ChunkManager::setPreviewSizes(Settings::previewSizeAudio() * 1024, Settings::previewSizeVideo() * 1024);
    bt::QueueManagerInterface::setQueueManagerEnabled(!Settings::manuallyControlTorrents());
    bt::Downloader::setUseWebSeeds(Settings::webseedsEnabled());
    bt::Peer::setResolveHostnames(Settings::lookUpHostnameOfPeers());
}

QString TorrentFileFilter(bool all_files_included)
{
    QString ret = i18nc("*.torrent", "Torrents") + QLatin1String(" (*.torrent)");
    if (all_files_included)
        ret += QLatin1String(";;") + i18n("All files") + QLatin1String(" (*)");
    return ret;
}

}
