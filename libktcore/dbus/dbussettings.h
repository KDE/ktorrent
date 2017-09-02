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

#ifndef __DBusSettings__
#define __DBusSettings__

#include <QObject>
#include <QList>
#include <ktcore_export.h>

namespace kt
{
    class CoreInterface;

    class KTCORE_EXPORT DBusSettings : public QObject
    {
        Q_OBJECT
        Q_CLASSINFO("D-Bus Interface", "org.ktorrent.settings")
    public:
        DBusSettings(CoreInterface* core, QObject* parent);
        ~DBusSettings();

    public slots:
        Q_SCRIPTABLE int maxDownloads();
        Q_SCRIPTABLE void setMaxDownloads(int val);
        Q_SCRIPTABLE int maxSeeds();
        Q_SCRIPTABLE void setMaxSeeds(int val);
        Q_SCRIPTABLE int startDownloadsOnLowDiskSpace();
        Q_SCRIPTABLE void setStartDownloadsOnLowDiskSpace(int val);
        Q_SCRIPTABLE int maxConnections();
        Q_SCRIPTABLE void setMaxConnections(int val);
        Q_SCRIPTABLE int maxTotalConnections();
        Q_SCRIPTABLE void setMaxTotalConnections(int val);
        Q_SCRIPTABLE int maxUploadRate();
        Q_SCRIPTABLE void setMaxUploadRate(int val);
        Q_SCRIPTABLE int maxDownloadRate();
        Q_SCRIPTABLE void setMaxDownloadRate(int val);
        Q_SCRIPTABLE double maxRatio();
        Q_SCRIPTABLE void setMaxRatio(double val);
        Q_SCRIPTABLE double greenRatio();
        Q_SCRIPTABLE void setGreenRatio(double val);
        Q_SCRIPTABLE int port();
        Q_SCRIPTABLE void setPort(int val);
        Q_SCRIPTABLE int udpTrackerPort();
        Q_SCRIPTABLE void setUdpTrackerPort(int val);
        Q_SCRIPTABLE bool showSystemTrayIcon();
        Q_SCRIPTABLE void setShowSystemTrayIcon(bool val);
        Q_SCRIPTABLE bool showSpeedBarInTrayIcon();
        Q_SCRIPTABLE void setShowSpeedBarInTrayIcon(bool val);
        Q_SCRIPTABLE int downloadBandwidth();
        Q_SCRIPTABLE void setDownloadBandwidth(int val);
        Q_SCRIPTABLE int uploadBandwidth();
        Q_SCRIPTABLE void setUploadBandwidth(int val);
        Q_SCRIPTABLE bool showPopups();
        Q_SCRIPTABLE void setShowPopups(bool val);
        Q_SCRIPTABLE bool keepSeeding();
        Q_SCRIPTABLE void setKeepSeeding(bool val);
        Q_SCRIPTABLE QString tempDir();
        Q_SCRIPTABLE void setTempDir(QString val);
        Q_SCRIPTABLE bool useSaveDir();
        Q_SCRIPTABLE void setUseSaveDir(bool val);
        Q_SCRIPTABLE QString saveDir();
        Q_SCRIPTABLE void setSaveDir(QString val);
        Q_SCRIPTABLE bool useTorrentCopyDir();
        Q_SCRIPTABLE void setUseTorrentCopyDir(bool val);
        Q_SCRIPTABLE QString torrentCopyDir();
        Q_SCRIPTABLE void setTorrentCopyDir(QString val);
        Q_SCRIPTABLE bool useCustomIP();
        Q_SCRIPTABLE void setUseCustomIP(bool val);
        Q_SCRIPTABLE QString lastSaveDir();
        Q_SCRIPTABLE void setLastSaveDir(QString val);
        Q_SCRIPTABLE QString customIP();
        Q_SCRIPTABLE void setCustomIP(QString val);
        Q_SCRIPTABLE int guiUpdateInterval();
        Q_SCRIPTABLE void setGuiUpdateInterval(int val);
        Q_SCRIPTABLE bool dhtSupport();
        Q_SCRIPTABLE void setDhtSupport(bool val);
        Q_SCRIPTABLE int dhtPort();
        Q_SCRIPTABLE void setDhtPort(int val);
        Q_SCRIPTABLE bool pexEnabled();
        Q_SCRIPTABLE void setPexEnabled(bool val);
        Q_SCRIPTABLE int numUploadSlots();
        Q_SCRIPTABLE void setNumUploadSlots(int val);
        Q_SCRIPTABLE bool useEncryption();
        Q_SCRIPTABLE void setUseEncryption(bool val);
        Q_SCRIPTABLE bool allowUnencryptedConnections();
        Q_SCRIPTABLE void setAllowUnencryptedConnections(bool val);
        Q_SCRIPTABLE int typeOfService();
        Q_SCRIPTABLE void setTypeOfService(int val);
        Q_SCRIPTABLE int dscp();
        Q_SCRIPTABLE void setDscp(int val);
        Q_SCRIPTABLE int maxConnectingSockets();
        Q_SCRIPTABLE void setMaxConnectingSockets(int val);
        Q_SCRIPTABLE bool checkWhenFinished();
        Q_SCRIPTABLE void setCheckWhenFinished(bool val);
        Q_SCRIPTABLE QList<int> shownColumns();
        Q_SCRIPTABLE void setShownColumns(QList<int> val);
        Q_SCRIPTABLE bool useKDEProxySettings();
        Q_SCRIPTABLE void setUseKDEProxySettings(bool val);
        Q_SCRIPTABLE QString httpProxy();
        Q_SCRIPTABLE void setHttpProxy(QString val);
        Q_SCRIPTABLE int httpProxyPort();
        Q_SCRIPTABLE void setHttpProxyPort(int val);
        Q_SCRIPTABLE bool useProxyForWebSeeds();
        Q_SCRIPTABLE void setUseProxyForWebSeeds(bool val);
        Q_SCRIPTABLE bool useProxyForTracker();
        Q_SCRIPTABLE void setUseProxyForTracker(bool val);
        Q_SCRIPTABLE bool socksEnabled();
        Q_SCRIPTABLE void setSocksEnabled(bool val);
        Q_SCRIPTABLE QString socksProxy();
        Q_SCRIPTABLE void setSocksProxy(QString val);
        Q_SCRIPTABLE int socksPort();
        Q_SCRIPTABLE void setSocksPort(int val);
        Q_SCRIPTABLE int socksVersion();
        Q_SCRIPTABLE void setSocksVersion(int val);
        Q_SCRIPTABLE bool socksUsePassword();
        Q_SCRIPTABLE void setSocksUsePassword(bool val);
        Q_SCRIPTABLE QString socksUsername();
        Q_SCRIPTABLE void setSocksUsername(QString val);
        Q_SCRIPTABLE QString socksPassword();
        Q_SCRIPTABLE void setSocksPassword(QString val);
        Q_SCRIPTABLE bool diskPrealloc();
        Q_SCRIPTABLE void setDiskPrealloc(bool val);
        Q_SCRIPTABLE bool fullDiskPrealloc();
        Q_SCRIPTABLE void setFullDiskPrealloc(bool val);
        Q_SCRIPTABLE int minDiskSpace();
        Q_SCRIPTABLE void setMinDiskSpace(int val);
        Q_SCRIPTABLE int cpuUsage();
        Q_SCRIPTABLE void setCpuUsage(int val);
        Q_SCRIPTABLE bool useCompletedDir();
        Q_SCRIPTABLE void setUseCompletedDir(bool val);
        Q_SCRIPTABLE QString completedDir();
        Q_SCRIPTABLE void setCompletedDir(QString val);
        Q_SCRIPTABLE double maxSeedTime();
        Q_SCRIPTABLE void setMaxSeedTime(double val);
        Q_SCRIPTABLE QString networkInterface();
        Q_SCRIPTABLE void setNetworkInterface(const QString &val);
        Q_SCRIPTABLE bool openMultipleTorrentsSilently();
        Q_SCRIPTABLE void setOpenMultipleTorrentsSilently(bool val);
        Q_SCRIPTABLE bool openAllTorrentsSilently();
        Q_SCRIPTABLE void setOpenAllTorrentsSilently(bool val);
        Q_SCRIPTABLE bool decreasePriorityOfStalledTorrents();
        Q_SCRIPTABLE void setDecreasePriorityOfStalledTorrents(bool val);
        Q_SCRIPTABLE int stallTimer();
        Q_SCRIPTABLE void setStallTimer(int val);
        Q_SCRIPTABLE int previewSizeAudio();
        Q_SCRIPTABLE void setPreviewSizeAudio(int val);
        Q_SCRIPTABLE int previewSizeVideo();
        Q_SCRIPTABLE void setPreviewSizeVideo(int val);
        Q_SCRIPTABLE bool suppressSleep();
        Q_SCRIPTABLE void setSuppressSleep(bool val);
        Q_SCRIPTABLE bool manuallyControlTorrents();
        Q_SCRIPTABLE void setManuallyControlTorrents(bool val);
        Q_SCRIPTABLE bool webseedsEnabled();
        Q_SCRIPTABLE void setWebseedsEnabled(bool val);
        Q_SCRIPTABLE bool useQHttpAnnounce();
        Q_SCRIPTABLE void setUseQHttpAnnounce(bool val);
        Q_SCRIPTABLE bool lookUpHostnameOfPeers();
        Q_SCRIPTABLE void setLookUpHostnameOfPeers(bool val);
        Q_SCRIPTABLE bool utpEnabled();
        Q_SCRIPTABLE void setUtpEnabled(bool val);
        Q_SCRIPTABLE bool onlyUseUtp();
        Q_SCRIPTABLE void setOnlyUseUtp(bool val);
        Q_SCRIPTABLE int primaryTransportProtocol();
        Q_SCRIPTABLE void setPrimaryTransportProtocol(int val);
        Q_SCRIPTABLE bool autoRenameSingleFileTorrents();
        Q_SCRIPTABLE void setAutoRenameSingleFileTorrents(bool val);
        Q_SCRIPTABLE int numMagnetDownloadingSlots();
        Q_SCRIPTABLE void setNumMagnetDownloadingSlots(int val);
        Q_SCRIPTABLE bool requeueMagnets();
        Q_SCRIPTABLE void setRequeueMagnets(bool val);
        Q_SCRIPTABLE int requeueMagnetsTime();
        Q_SCRIPTABLE void setRequeueMagnetsTime(int val);
        Q_SCRIPTABLE bool showTotalSpeedInTitle();
        Q_SCRIPTABLE void setShowTotalSpeedInTitle(bool val);

        Q_SCRIPTABLE void apply();
    private:
        CoreInterface* core;
    };
}

#endif
