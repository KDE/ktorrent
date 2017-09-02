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

#include "dbussettings.h"
#include "settings.h"

#include <QDBusConnection>

#include <interfaces/coreinterface.h>

namespace kt
{
    DBusSettings::DBusSettings(CoreInterface* core, QObject* parent) : QObject(parent), core(core)
    {
        QDBusConnection::sessionBus().registerObject(QStringLiteral("/settings"), this,
                QDBusConnection::ExportScriptableSlots | QDBusConnection::ExportScriptableSignals);
    }

    DBusSettings::~DBusSettings()
    {}

    void DBusSettings::apply()
    {
        core->applySettings();
    }

    int DBusSettings::maxDownloads()
    {
        return Settings::maxDownloads();
    }

    void DBusSettings::setMaxDownloads(int val)
    {
        Settings::setMaxDownloads(val);
    }

    int DBusSettings::maxSeeds()
    {
        return Settings::maxSeeds();
    }

    void DBusSettings::setMaxSeeds(int val)
    {
        Settings::setMaxSeeds(val);
    }

    int DBusSettings::startDownloadsOnLowDiskSpace()
    {
        return Settings::startDownloadsOnLowDiskSpace();
    }

    void DBusSettings::setStartDownloadsOnLowDiskSpace(int val)
    {
        Settings::setStartDownloadsOnLowDiskSpace(val);
    }

    int DBusSettings::maxConnections()
    {
        return Settings::maxConnections();
    }

    void DBusSettings::setMaxConnections(int val)
    {
        Settings::setMaxConnections(val);
    }

    int DBusSettings::maxTotalConnections()
    {
        return Settings::maxTotalConnections();
    }

    void DBusSettings::setMaxTotalConnections(int val)
    {
        Settings::setMaxTotalConnections(val);
    }

    int DBusSettings::maxUploadRate()
    {
        return Settings::maxUploadRate();
    }

    void DBusSettings::setMaxUploadRate(int val)
    {
        Settings::setMaxUploadRate(val);
    }

    int DBusSettings::maxDownloadRate()
    {
        return Settings::maxDownloadRate();
    }

    void DBusSettings::setMaxDownloadRate(int val)
    {
        Settings::setMaxDownloadRate(val);
    }

    double DBusSettings::maxRatio()
    {
        return Settings::maxRatio();
    }

    void DBusSettings::setMaxRatio(double val)
    {
        Settings::setMaxRatio(val);
    }

    double DBusSettings::greenRatio()
    {
        return Settings::greenRatio();
    }

    void DBusSettings::setGreenRatio(double val)
    {
        Settings::setGreenRatio(val);
    }

    int DBusSettings::port()
    {
        return Settings::port();
    }

    void DBusSettings::setPort(int val)
    {
        Settings::setPort(val);
    }

    int DBusSettings::udpTrackerPort()
    {
        return Settings::udpTrackerPort();
    }

    void DBusSettings::setUdpTrackerPort(int val)
    {
        Settings::setUdpTrackerPort(val);
    }

    bool DBusSettings::showSystemTrayIcon()
    {
        return Settings::showSystemTrayIcon();
    }

    void DBusSettings::setShowSystemTrayIcon(bool val)
    {
        Settings::setShowSystemTrayIcon(val);
    }

    bool DBusSettings::showSpeedBarInTrayIcon()
    {
        return Settings::showSpeedBarInTrayIcon();
    }

    void DBusSettings::setShowSpeedBarInTrayIcon(bool val)
    {
        Settings::setShowSpeedBarInTrayIcon(val);
    }

    int DBusSettings::downloadBandwidth()
    {
        return Settings::downloadBandwidth();
    }

    void DBusSettings::setDownloadBandwidth(int val)
    {
        Settings::setDownloadBandwidth(val);
    }

    int DBusSettings::uploadBandwidth()
    {
        return Settings::uploadBandwidth();
    }

    void DBusSettings::setUploadBandwidth(int val)
    {
        Settings::setUploadBandwidth(val);
    }

    bool DBusSettings::showPopups()
    {
        return Settings::showPopups();
    }

    void DBusSettings::setShowPopups(bool val)
    {
        Settings::setShowPopups(val);
    }

    bool DBusSettings::keepSeeding()
    {
        return Settings::keepSeeding();
    }

    void DBusSettings::setKeepSeeding(bool val)
    {
        Settings::setKeepSeeding(val);
    }

    QString DBusSettings::tempDir()
    {
        return Settings::tempDir();
    }

    void DBusSettings::setTempDir(QString val)
    {
        Settings::setTempDir(val);
    }

    bool DBusSettings::useSaveDir()
    {
        return Settings::useSaveDir();
    }

    void DBusSettings::setUseSaveDir(bool val)
    {
        Settings::setUseSaveDir(val);
    }

    QString DBusSettings::saveDir()
    {
        return Settings::saveDir();
    }

    void DBusSettings::setSaveDir(QString val)
    {
        Settings::setSaveDir(val);
    }

    bool DBusSettings::useTorrentCopyDir()
    {
        return Settings::useTorrentCopyDir();
    }

    void DBusSettings::setUseTorrentCopyDir(bool val)
    {
        Settings::setUseTorrentCopyDir(val);
    }

    QString DBusSettings::torrentCopyDir()
    {
        return Settings::torrentCopyDir();
    }

    void DBusSettings::setTorrentCopyDir(QString val)
    {
        Settings::setTorrentCopyDir(val);
    }

    bool DBusSettings::useCustomIP()
    {
        return Settings::useCustomIP();
    }

    void DBusSettings::setUseCustomIP(bool val)
    {
        Settings::setUseCustomIP(val);
    }

    QString DBusSettings::lastSaveDir()
    {
        return Settings::lastSaveDir();
    }

    void DBusSettings::setLastSaveDir(QString val)
    {
        Settings::setLastSaveDir(val);
    }

    QString DBusSettings::customIP()
    {
        return Settings::customIP();
    }

    void DBusSettings::setCustomIP(QString val)
    {
        Settings::setCustomIP(val);
    }

    int DBusSettings::guiUpdateInterval()
    {
        return Settings::guiUpdateInterval();
    }

    void DBusSettings::setGuiUpdateInterval(int val)
    {
        Settings::setGuiUpdateInterval(val);
    }

    bool DBusSettings::dhtSupport()
    {
        return Settings::dhtSupport();
    }

    void DBusSettings::setDhtSupport(bool val)
    {
        Settings::setDhtSupport(val);
    }

    int DBusSettings::dhtPort()
    {
        return Settings::dhtPort();
    }

    void DBusSettings::setDhtPort(int val)
    {
        Settings::setDhtPort(val);
    }

    bool DBusSettings::pexEnabled()
    {
        return Settings::pexEnabled();
    }

    void DBusSettings::setPexEnabled(bool val)
    {
        Settings::setPexEnabled(val);
    }

    int DBusSettings::numUploadSlots()
    {
        return Settings::numUploadSlots();
    }

    void DBusSettings::setNumUploadSlots(int val)
    {
        Settings::setNumUploadSlots(val);
    }

    bool DBusSettings::useEncryption()
    {
        return Settings::useEncryption();
    }

    void DBusSettings::setUseEncryption(bool val)
    {
        Settings::setUseEncryption(val);
    }

    bool DBusSettings::allowUnencryptedConnections()
    {
        return Settings::allowUnencryptedConnections();
    }

    void DBusSettings::setAllowUnencryptedConnections(bool val)
    {
        Settings::setAllowUnencryptedConnections(val);
    }

    int DBusSettings::typeOfService()
    {
        return Settings::typeOfService();
    }

    void DBusSettings::setTypeOfService(int val)
    {
        Settings::setTypeOfService(val);
    }

    int DBusSettings::dscp()
    {
        return Settings::dscp();
    }

    void DBusSettings::setDscp(int val)
    {
        Settings::setDscp(val);
    }

    int DBusSettings::maxConnectingSockets()
    {
        return Settings::maxConnectingSockets();
    }

    void DBusSettings::setMaxConnectingSockets(int val)
    {
        Settings::setMaxConnectingSockets(val);
    }

    bool DBusSettings::checkWhenFinished()
    {
        return Settings::checkWhenFinished();
    }

    void DBusSettings::setCheckWhenFinished(bool val)
    {
        Settings::setCheckWhenFinished(val);
    }

    QList<int> DBusSettings::shownColumns()
    {
        return Settings::shownColumns();
    }

    void DBusSettings::setShownColumns(QList<int> val)
    {
        Settings::setShownColumns(val);
    }

    bool DBusSettings::useKDEProxySettings()
    {
        return Settings::useKDEProxySettings();
    }

    void DBusSettings::setUseKDEProxySettings(bool val)
    {
        Settings::setUseKDEProxySettings(val);
    }

    QString DBusSettings::httpProxy()
    {
        return Settings::httpProxy();
    }

    void DBusSettings::setHttpProxy(QString val)
    {
        Settings::setHttpProxy(val);
    }

    int DBusSettings::httpProxyPort()
    {
        return Settings::httpProxyPort();
    }

    void DBusSettings::setHttpProxyPort(int val)
    {
        Settings::setHttpProxyPort(val);
    }

    bool DBusSettings::useProxyForWebSeeds()
    {
        return Settings::useProxyForWebSeeds();
    }

    void DBusSettings::setUseProxyForWebSeeds(bool val)
    {
        Settings::setUseProxyForWebSeeds(val);
    }

    bool DBusSettings::useProxyForTracker()
    {
        return Settings::useProxyForTracker();
    }

    void DBusSettings::setUseProxyForTracker(bool val)
    {
        Settings::setUseProxyForTracker(val);
    }

    bool DBusSettings::socksEnabled()
    {
        return Settings::socksEnabled();
    }

    void DBusSettings::setSocksEnabled(bool val)
    {
        Settings::setSocksEnabled(val);
    }

    QString DBusSettings::socksProxy()
    {
        return Settings::socksProxy();
    }

    void DBusSettings::setSocksProxy(QString val)
    {
        Settings::setSocksProxy(val);
    }

    int DBusSettings::socksPort()
    {
        return Settings::socksPort();
    }

    void DBusSettings::setSocksPort(int val)
    {
        Settings::setSocksPort(val);
    }

    int DBusSettings::socksVersion()
    {
        return Settings::socksVersion();
    }

    void DBusSettings::setSocksVersion(int val)
    {
        Settings::setSocksVersion(val);
    }

    bool DBusSettings::socksUsePassword()
    {
        return Settings::socksUsePassword();
    }

    void DBusSettings::setSocksUsePassword(bool val)
    {
        Settings::setSocksUsePassword(val);
    }

    QString DBusSettings::socksUsername()
    {
        return Settings::socksUsername();
    }

    void DBusSettings::setSocksUsername(QString val)
    {
        Settings::setSocksUsername(val);
    }

    QString DBusSettings::socksPassword()
    {
        return Settings::socksPassword();
    }

    void DBusSettings::setSocksPassword(QString val)
    {
        Settings::setSocksPassword(val);
    }

    bool DBusSettings::diskPrealloc()
    {
        return Settings::diskPrealloc();
    }

    void DBusSettings::setDiskPrealloc(bool val)
    {
        Settings::setDiskPrealloc(val);
    }

    bool DBusSettings::fullDiskPrealloc()
    {
        return Settings::fullDiskPrealloc();
    }

    void DBusSettings::setFullDiskPrealloc(bool val)
    {
        Settings::setFullDiskPrealloc(val);
    }

    int DBusSettings::minDiskSpace()
    {
        return Settings::minDiskSpace();
    }

    void DBusSettings::setMinDiskSpace(int val)
    {
        Settings::setMinDiskSpace(val);
    }

    int DBusSettings::cpuUsage()
    {
        return Settings::cpuUsage();
    }

    void DBusSettings::setCpuUsage(int val)
    {
        Settings::setCpuUsage(val);
    }

    bool DBusSettings::useCompletedDir()
    {
        return Settings::useCompletedDir();
    }

    void DBusSettings::setUseCompletedDir(bool val)
    {
        Settings::setUseCompletedDir(val);
    }

    QString DBusSettings::completedDir()
    {
        return Settings::completedDir();
    }

    void DBusSettings::setCompletedDir(QString val)
    {
        Settings::setCompletedDir(val);
    }

    double DBusSettings::maxSeedTime()
    {
        return Settings::maxSeedTime();
    }

    void DBusSettings::setMaxSeedTime(double val)
    {
        Settings::setMaxSeedTime(val);
    }

    QString DBusSettings::networkInterface()
    {
        return Settings::networkInterface();
    }

    void DBusSettings::setNetworkInterface(const QString &val)
    {
        Settings::setNetworkInterface(val);
    }

    bool DBusSettings::openMultipleTorrentsSilently()
    {
        return Settings::openMultipleTorrentsSilently();
    }

    void DBusSettings::setOpenMultipleTorrentsSilently(bool val)
    {
        Settings::setOpenMultipleTorrentsSilently(val);
    }

    bool DBusSettings::openAllTorrentsSilently()
    {
        return Settings::openAllTorrentsSilently();
    }

    void DBusSettings::setOpenAllTorrentsSilently(bool val)
    {
        Settings::setOpenAllTorrentsSilently(val);
    }

    bool DBusSettings::decreasePriorityOfStalledTorrents()
    {
        return Settings::decreasePriorityOfStalledTorrents();
    }

    void DBusSettings::setDecreasePriorityOfStalledTorrents(bool val)
    {
        Settings::setDecreasePriorityOfStalledTorrents(val);
    }

    int DBusSettings::stallTimer()
    {
        return Settings::stallTimer();
    }

    void DBusSettings::setStallTimer(int val)
    {
        Settings::setStallTimer(val);
    }

    int DBusSettings::previewSizeAudio()
    {
        return Settings::previewSizeAudio();
    }

    void DBusSettings::setPreviewSizeAudio(int val)
    {
        Settings::setPreviewSizeAudio(val);
    }

    int DBusSettings::previewSizeVideo()
    {
        return Settings::previewSizeVideo();
    }

    void DBusSettings::setPreviewSizeVideo(int val)
    {
        Settings::setPreviewSizeVideo(val);
    }

    bool DBusSettings::suppressSleep()
    {
        return Settings::suppressSleep();
    }

    void DBusSettings::setSuppressSleep(bool val)
    {
        Settings::setSuppressSleep(val);
    }

    bool DBusSettings::manuallyControlTorrents()
    {
        return Settings::manuallyControlTorrents();
    }

    void DBusSettings::setManuallyControlTorrents(bool val)
    {
        Settings::setManuallyControlTorrents(val);
    }

    bool DBusSettings::webseedsEnabled()
    {
        return Settings::webseedsEnabled();
    }

    void DBusSettings::setWebseedsEnabled(bool val)
    {
        Settings::setWebseedsEnabled(val);
    }

    bool DBusSettings::useQHttpAnnounce()
    {
        return Settings::useQHttpAnnounce();
    }

    void DBusSettings::setUseQHttpAnnounce(bool val)
    {
        Settings::setUseQHttpAnnounce(val);
    }

    bool DBusSettings::lookUpHostnameOfPeers()
    {
        return Settings::lookUpHostnameOfPeers();
    }

    void DBusSettings::setLookUpHostnameOfPeers(bool val)
    {
        Settings::setLookUpHostnameOfPeers(val);
    }

    bool DBusSettings::utpEnabled()
    {
        return Settings::utpEnabled();
    }

    void DBusSettings::setUtpEnabled(bool val)
    {
        Settings::setUtpEnabled(val);
    }

    bool DBusSettings::onlyUseUtp()
    {
        return Settings::onlyUseUtp();
    }

    void DBusSettings::setOnlyUseUtp(bool val)
    {
        Settings::setOnlyUseUtp(val);
    }

    int DBusSettings::primaryTransportProtocol()
    {
        return Settings::primaryTransportProtocol();
    }

    void DBusSettings::setPrimaryTransportProtocol(int val)
    {
        Settings::setPrimaryTransportProtocol(val);
    }

    bool DBusSettings::autoRenameSingleFileTorrents()
    {
        return Settings::autoRenameSingleFileTorrents();
    }

    void DBusSettings::setAutoRenameSingleFileTorrents(bool val)
    {
        Settings::setAutoRenameSingleFileTorrents(val);
    }

    int DBusSettings::numMagnetDownloadingSlots()
    {
        return Settings::numMagnetDownloadingSlots();
    }

    void DBusSettings::setNumMagnetDownloadingSlots(int val)
    {
        Settings::setNumMagnetDownloadingSlots(val);
    }

    bool DBusSettings::requeueMagnets()
    {
        return Settings::requeueMagnets();
    }
    bool DBusSettings::showTotalSpeedInTitle()
    {
        return Settings::showTotalSpeedInTitle();
    }

    void DBusSettings::setShowTotalSpeedInTitle(bool val)
    {
        Settings::setShowTotalSpeedInTitle(val);
    }

    void DBusSettings::setRequeueMagnets(bool val)
    {
        Settings::setRequeueMagnets(val);
    }

    int DBusSettings::requeueMagnetsTime()
    {
        return Settings::requeueMagnetsTime();
    }

    void DBusSettings::setRequeueMagnetsTime(int val)
    {
        Settings::setRequeueMagnetsTime(val);
    }
}
