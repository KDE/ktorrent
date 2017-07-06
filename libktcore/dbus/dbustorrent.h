/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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

#ifndef KTDBUSTORRENT_H
#define KTDBUSTORRENT_H

#include <QObject>
#include <QStringList>

#include <util/constants.h>
#include <interfaces/torrentinterface.h>


namespace kt
{

    class DBusTorrentFileStream;

    /**
        DBus object to access a torrent
    */
    class DBusTorrent : public QObject
    {
        Q_OBJECT
        Q_CLASSINFO("D-Bus Interface", "org.ktorrent.torrent")
    public:
        DBusTorrent(bt::TorrentInterface* ti, QObject* parent);
        ~DBusTorrent();

        /// Get a pointer to the actual torrent
        bt::TorrentInterface* torrent() {return ti;}

    public Q_SLOTS:
        Q_SCRIPTABLE QString infoHash() const;
        Q_SCRIPTABLE QString name() const;
        Q_SCRIPTABLE bool isPrivate() const;

        // Speed
        Q_SCRIPTABLE uint downloadSpeed() const;
        Q_SCRIPTABLE uint uploadSpeed() const;

        // Data stuff
        Q_SCRIPTABLE qulonglong bytesDownloaded() const;
        Q_SCRIPTABLE qulonglong bytesUploaded() const;
        Q_SCRIPTABLE qulonglong totalSize() const;
        Q_SCRIPTABLE qulonglong bytesLeftToDownload() const;
        Q_SCRIPTABLE qulonglong bytesLeft() const;
        Q_SCRIPTABLE qulonglong bytesToDownload() const;

        // Priority and QM
        Q_SCRIPTABLE int priority() const;
        Q_SCRIPTABLE void setPriority(int p);
        Q_SCRIPTABLE void setAllowedToStart(bool on);
        Q_SCRIPTABLE bool isAllowedToStart() const;

        // Chunks
        Q_SCRIPTABLE uint chunks() const;
        Q_SCRIPTABLE uint chunkSize() const;
        Q_SCRIPTABLE bool chunkDownloaded(uint idx) const;

        // Seeders and leechers
        Q_SCRIPTABLE uint seedersConnected() const;
        Q_SCRIPTABLE uint seedersTotal() const;
        Q_SCRIPTABLE uint leechersConnected() const;
        Q_SCRIPTABLE uint leechersTotal() const;

        // Tracker stuff
        Q_SCRIPTABLE QString currentTracker() const;
        Q_SCRIPTABLE QStringList trackers() const;
        Q_SCRIPTABLE void setTrackerEnabled(const QString& tracker_url, bool enabled);
        Q_SCRIPTABLE void changeTracker(const QString& tracker_url);
        Q_SCRIPTABLE void announce();
        Q_SCRIPTABLE void scrape();
        Q_SCRIPTABLE bool addTracker(const QString& tracker_url);
        Q_SCRIPTABLE bool removeTracker(const QString& tracker_url);
        Q_SCRIPTABLE void restoreDefaultTrackers();

        // WebSeed stuff
        Q_SCRIPTABLE QStringList webSeeds() const;
        Q_SCRIPTABLE bool addWebSeed(const QString& webseed_url);
        Q_SCRIPTABLE bool removeWebSeed(const QString& webseed_url);

        // Files
        Q_SCRIPTABLE uint numFiles() const;
        Q_SCRIPTABLE QString dataDir() const;
        Q_SCRIPTABLE QString torDir() const;
        Q_SCRIPTABLE QString pathOnDisk() const;
        Q_SCRIPTABLE QString filePath(uint file_index) const;
        Q_SCRIPTABLE QString filePathOnDisk(uint file_index) const;
        Q_SCRIPTABLE qulonglong fileSize(uint file_index) const;
        Q_SCRIPTABLE int filePriority(uint file_index) const;
        Q_SCRIPTABLE void setFilePriority(uint file_index, int prio);
        Q_SCRIPTABLE int firstChunkOfFile(uint file_index) const;
        Q_SCRIPTABLE int lastChunkOfFile(uint file_index) const;
        Q_SCRIPTABLE double filePercentage(uint file_index) const;
        Q_SCRIPTABLE bool isMultiMediaFile(uint file_index) const;
        Q_SCRIPTABLE void setDoNotDownload(uint file_index, bool dnd);

        // Stats
        Q_SCRIPTABLE QByteArray stats() const;

        // Max share ratio and seed time
        Q_SCRIPTABLE void setMaxShareRatio(double ratio);
        Q_SCRIPTABLE double maxShareRatio() const;
        Q_SCRIPTABLE double shareRatio() const;
        Q_SCRIPTABLE void setMaxSeedTime(double hours);
        Q_SCRIPTABLE double maxSeedTime() const;
        Q_SCRIPTABLE double seedTime() const;

        Q_SCRIPTABLE bool createStream(uint file_index);
        Q_SCRIPTABLE bool removeStream(uint file_index);

    Q_SIGNALS:
        void finished(QObject* tor);
        void stoppedByError(QObject* tor, const QString& msg);
        void seedingAutoStopped(QObject* tor, const QString& reason);
        void corruptedDataFound(QObject* tor);
        void torrentStopped(QObject* tor);

    private slots:
        void onFinished(bt::TorrentInterface* tor);
        void onStoppedByError(bt::TorrentInterface* tor, const QString& err);
        void onSeedingAutoStopped(bt::TorrentInterface* tor, bt::AutoStopReason reason);
        void onCorruptedDataFound(bt::TorrentInterface* tor);
        void onTorrentStopped(bt::TorrentInterface* tor);

    private:
        bt::TorrentInterface* ti;
        DBusTorrentFileStream* stream;
    };

}

#endif
