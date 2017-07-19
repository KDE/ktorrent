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

#include <QDBusConnection>
#include <QThread>

#include <KLocalizedString>

#include <util/log.h>
#include <util/sha1hash.h>
#include <util/bitset.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/torrentfileinterface.h>
#include <interfaces/trackerslist.h>
#include <interfaces/webseedinterface.h>
#include <bcodec/bencoder.h>
#include "dbustorrent.h"
#include <interfaces/trackerinterface.h>
#include "dbustorrentfilestream.h"

using namespace bt;

namespace kt
{


    DBusTorrent::DBusTorrent(bt::TorrentInterface* ti, QObject* parent)
        : QObject(parent), ti(ti), stream(0)
    {
        QDBusConnection sb = QDBusConnection::sessionBus();
        QString path = QLatin1String("/torrent/") + ti->getInfoHash().toString();
        QFlags<QDBusConnection::RegisterOption> flags = QDBusConnection::ExportScriptableSlots | QDBusConnection::ExportScriptableSignals;
        sb.registerObject(path, this, flags);

        connect(ti, &bt::TorrentInterface::finished, this, &DBusTorrent::onFinished);
        connect(ti, &bt::TorrentInterface::stoppedByError, this, &DBusTorrent::onStoppedByError);
        connect(ti, &bt::TorrentInterface::seedingAutoStopped, this, &DBusTorrent::onSeedingAutoStopped);
        connect(ti, &bt::TorrentInterface::corruptedDataFound, this, &DBusTorrent::onCorruptedDataFound);
        connect(ti, &bt::TorrentInterface::torrentStopped, this, &DBusTorrent::onTorrentStopped);
    }


    DBusTorrent::~DBusTorrent()
    {
    }

    QString DBusTorrent::infoHash() const
    {
        const bt::SHA1Hash& h = ti->getInfoHash();
        return h.toString();
    }

    QString DBusTorrent::name() const
    {
        return ti->getDisplayName();
    }

    bool DBusTorrent::isPrivate() const
    {
        return ti->getStats().priv_torrent;
    }

    uint DBusTorrent::downloadSpeed() const
    {
        return ti->getStats().download_rate;
    }

    uint DBusTorrent::uploadSpeed() const
    {
        return ti->getStats().upload_rate;
    }

    qulonglong DBusTorrent::bytesDownloaded() const
    {
        return ti->getStats().bytes_downloaded;
    }

    qulonglong DBusTorrent::bytesUploaded() const
    {
        return ti->getStats().bytes_uploaded;
    }

    qulonglong DBusTorrent::totalSize() const
    {
        return ti->getStats().total_bytes;
    }

    qulonglong DBusTorrent::bytesLeftToDownload() const
    {
        return ti->getStats().bytes_left_to_download;
    }

    qulonglong DBusTorrent::bytesLeft() const
    {
        return ti->getStats().bytes_left;
    }

    qulonglong DBusTorrent::bytesToDownload() const
    {
        return ti->getStats().total_bytes_to_download;
    }

    uint DBusTorrent::chunks() const
    {
        return ti->getStats().total_chunks;
    }

    uint DBusTorrent::chunkSize() const
    {
        return ti->getStats().chunk_size;
    }

    bool DBusTorrent::chunkDownloaded(uint idx) const
    {
        return ti->downloadedChunksBitSet().get(idx);
    }

    uint DBusTorrent::seedersConnected() const
    {
        return ti->getStats().seeders_connected_to;
    }

    uint DBusTorrent::seedersTotal() const
    {
        return ti->getStats().seeders_total;
    }

    uint DBusTorrent::leechersConnected() const
    {
        return ti->getStats().leechers_connected_to;
    }

    uint DBusTorrent::leechersTotal() const
    {
        return ti->getStats().leechers_total;
    }

    QString DBusTorrent::currentTracker() const
    {
        bt::TrackerInterface* t = ti->getTrackersList()->getCurrentTracker();
        return t ? t->trackerURL().toDisplayString() : QString();
    }

    QStringList DBusTorrent::trackers() const
    {
        const QList<bt::TrackerInterface*> trackers = ti->getTrackersList()->getTrackers();
        QStringList ret;
        for (bt::TrackerInterface* t : trackers)
            ret << t->trackerURL().toDisplayString();
        return ret;
    }

    void DBusTorrent::changeTracker(const QString& tracker_url)
    {
        QUrl url(tracker_url);
        ti->getTrackersList()->setCurrentTracker(url);
    }

    void DBusTorrent::announce()
    {
        ti->updateTracker();
    }

    void DBusTorrent::scrape()
    {
        ti->scrapeTracker();
    }

    void DBusTorrent::setTrackerEnabled(const QString& tracker_url, bool enabled)
    {
        ti->getTrackersList()->setTrackerEnabled(QUrl(tracker_url), enabled);
    }

    bool DBusTorrent::addTracker(const QString& tracker_url)
    {
        if (ti->getStats().priv_torrent)
            return false;

        return ti->getTrackersList()->addTracker(QUrl(tracker_url), true) != 0;
    }

    bool DBusTorrent::removeTracker(const QString& tracker_url)
    {
        if (ti->getStats().priv_torrent)
            return false;

        ti->getTrackersList()->removeTracker(QUrl(tracker_url));
        return true;
    }

    void DBusTorrent::restoreDefaultTrackers()
    {
        ti->getTrackersList()->restoreDefault();
        ti->updateTracker();
    }

    QStringList DBusTorrent::webSeeds() const
    {
        QStringList ws;
        for (Uint32 i = 0; i < ti->getNumWebSeeds(); i++)
        {
            const WebSeedInterface* wsi = ti->getWebSeed(i);
            ws << wsi->getUrl().toDisplayString();
        }
        return ws;
    }

    bool DBusTorrent::addWebSeed(const QString& webseed_url)
    {
        return ti->addWebSeed(QUrl(webseed_url));
    }

    bool DBusTorrent::removeWebSeed(const QString& webseed_url)
    {
        return ti->removeWebSeed(QUrl(webseed_url));
    }

    uint DBusTorrent::numFiles() const
    {
        return ti->getNumFiles();
    }

    QString DBusTorrent::dataDir() const
    {
        return ti->getDataDir();
    }

    QString DBusTorrent::pathOnDisk() const
    {
        return ti->getStats().output_path;
    }

    QString DBusTorrent::torDir() const
    {
        return ti->getTorDir();
    }

    QByteArray DBusTorrent::stats() const
    {
        QByteArray ret;
        BEncoder enc(new BEncoderBufferOutput(ret));
        const TorrentStats& s = ti->getStats();
        enc.beginDict();
        enc.write(QByteArrayLiteral("imported_bytes"), s.imported_bytes);
        enc.write(QByteArrayLiteral("bytes_downloaded"), s.bytes_downloaded);
        enc.write(QByteArrayLiteral("bytes_uploaded"), s.bytes_uploaded);
        enc.write(QByteArrayLiteral("bytes_left"), s.bytes_left);
        enc.write(QByteArrayLiteral("bytes_left_to_download"), s.bytes_left_to_download);
        enc.write(QByteArrayLiteral("total_bytes"), s.total_bytes);
        enc.write(QByteArrayLiteral("total_bytes_to_download"), s.total_bytes_to_download);
        enc.write(QByteArrayLiteral("download_rate"), s.download_rate);
        enc.write(QByteArrayLiteral("upload_rate"), s.upload_rate);
        enc.write(QByteArrayLiteral("num_peers"), s.num_peers);
        enc.write(QByteArrayLiteral("num_chunks_downloading"), s.num_chunks_downloading);
        enc.write(QByteArrayLiteral("total_chunks"), s.total_chunks);
        enc.write(QByteArrayLiteral("num_chunks_downloaded"), s.num_chunks_downloaded);
        enc.write(QByteArrayLiteral("num_chunks_excluded"), s.num_chunks_excluded);
        enc.write(QByteArrayLiteral("num_chunks_left"), s.num_chunks_left);
        enc.write(QByteArrayLiteral("chunk_size"), s.chunk_size);
        enc.write(QByteArrayLiteral("seeders_total"), s.seeders_total);
        enc.write(QByteArrayLiteral("seeders_connected_to"), s.seeders_connected_to);
        enc.write(QByteArrayLiteral("leechers_total"), s.leechers_total);
        enc.write(QByteArrayLiteral("leechers_connected_to"), s.leechers_connected_to);
        enc.write(QByteArrayLiteral("status"), s.statusToString().toUtf8());
        enc.write(QByteArrayLiteral("session_bytes_downloaded"), s.session_bytes_downloaded);
        enc.write(QByteArrayLiteral("session_bytes_uploaded"), s.session_bytes_uploaded);
        enc.write(QByteArrayLiteral("output_path"), s.output_path.toUtf8());
        enc.write(QByteArrayLiteral("running"), s.running);
        enc.write(QByteArrayLiteral("started"), s.started);
        enc.write(QByteArrayLiteral("multi_file_torrent"), s.multi_file_torrent);
        enc.write(QByteArrayLiteral("stopped_by_error"), s.stopped_by_error);
        enc.write(QByteArrayLiteral("max_share_ratio"), s.max_share_ratio);
        enc.write(QByteArrayLiteral("max_seed_time"), s.max_seed_time);
        enc.write(QByteArrayLiteral("num_corrupted_chunks"), s.num_corrupted_chunks);
        const bt::BitSet& bs = ti->downloadedChunksBitSet();
        enc.write(QByteArrayLiteral("downloaded_chunks"));
        enc.write(bs.getData(), bs.getNumBytes());
        const bt::BitSet& ebs = ti->excludedChunksBitSet();
        enc.write(QByteArrayLiteral("excluded_chunks"));
        enc.write(ebs.getData(), ebs.getNumBytes());
        enc.end();
        return ret;
    }

    void DBusTorrent::onFinished(bt::TorrentInterface* tor)
    {
        Q_UNUSED(tor);
        emit finished(this);
    }

    void DBusTorrent::onStoppedByError(bt::TorrentInterface* tor, const QString& err)
    {
        Q_UNUSED(tor);
        emit stoppedByError(this, err);
    }

    void DBusTorrent::onSeedingAutoStopped(bt::TorrentInterface* tor, bt::AutoStopReason reason)
    {
        Q_UNUSED(tor);
        QString msg;
        switch (reason)
        {
        case bt::MAX_RATIO_REACHED:
            msg = i18n("Maximum share ratio reached.");
            break;
        case bt::MAX_SEED_TIME_REACHED:
            msg = i18n("Maximum seed time reached.");
            break;
        }
        emit seedingAutoStopped(this, msg);
    }

    void DBusTorrent::onCorruptedDataFound(bt::TorrentInterface* tor)
    {
        Q_UNUSED(tor);
        emit corruptedDataFound(this);
    }

    void DBusTorrent::onTorrentStopped(bt::TorrentInterface* tor)
    {
        Q_UNUSED(tor);
        //emit torrentStopped(this); //TODO emit string representation of the torrent
    }

    QString DBusTorrent::filePath(uint file_index) const
    {
        if (file_index >= ti->getNumFiles())
            return QString();
        else
            return ti->getTorrentFile(file_index).getPath();
    }

    QString DBusTorrent::filePathOnDisk(uint file_index) const
    {
        if (file_index >= ti->getNumFiles())
            return QString();
        else
            return ti->getTorrentFile(file_index).getPathOnDisk();
    }

    qulonglong DBusTorrent::fileSize(uint file_index) const
    {
        if (file_index >= ti->getNumFiles())
            return 0;
        else
            return ti->getTorrentFile(file_index).getSize();
    }

    int DBusTorrent::filePriority(uint file_index) const
    {
        if (file_index >= ti->getNumFiles())
            return 0;
        else
            return ti->getTorrentFile(file_index).getPriority();
    }

    void DBusTorrent::setFilePriority(uint file_index, int prio)
    {
        if (file_index >= ti->getNumFiles())
            return;

        if (prio > 60 || prio < 10)
            return;

        if (prio % 10 != 0)
            return;

        ti->getTorrentFile(file_index).setPriority((bt::Priority)prio);
    }

    int DBusTorrent::firstChunkOfFile(uint file_index) const
    {
        if (file_index >= ti->getNumFiles())
            return 0;
        else
            return ti->getTorrentFile(file_index).getFirstChunk();
    }

    int DBusTorrent::lastChunkOfFile(uint file_index) const
    {
        if (file_index >= ti->getNumFiles())
            return 0;
        else
            return ti->getTorrentFile(file_index).getLastChunk();
    }

    double DBusTorrent::filePercentage(uint file_index) const
    {
        if (file_index >= ti->getNumFiles())
            return 0;
        else
            return ti->getTorrentFile(file_index).getDownloadPercentage();
    }

    bool DBusTorrent::isMultiMediaFile(uint file_index) const
    {
        if (file_index >= ti->getNumFiles())
            return false;
        else
            return ti->getTorrentFile(file_index).isMultimedia();
    }

    void DBusTorrent::setDoNotDownload(uint file_index, bool dnd)
    {
        if (file_index >= ti->getNumFiles())
            return;

        ti->getTorrentFile(file_index).setDoNotDownload(dnd);
    }

    int DBusTorrent::priority() const
    {
        return ti->getPriority();
    }

    void DBusTorrent::setPriority(int p)
    {
        ti->setPriority(p);
    }

    void DBusTorrent::setAllowedToStart(bool on)
    {
        ti->setAllowedToStart(on);
    }

    bool DBusTorrent::isAllowedToStart() const
    {
        return ti->isAllowedToStart();
    }

    double DBusTorrent::maxSeedTime() const
    {
        return ti->getMaxSeedTime();
    }

    double DBusTorrent::maxShareRatio() const
    {
        return ti->getMaxShareRatio();
    }

    void DBusTorrent::setMaxSeedTime(double hours)
    {
        ti->setMaxSeedTime(hours);
    }

    void DBusTorrent::setMaxShareRatio(double ratio)
    {
        ti->setMaxShareRatio(ratio);
    }

    double DBusTorrent::seedTime() const
    {
        Uint32 dl = ti->getRunningTimeDL();
        Uint32 ul = ti->getRunningTimeUL();
        return (double)(ul - dl) / 3600.0;
    }

    double DBusTorrent::shareRatio() const
    {
        return ti->getStats().shareRatio();
    }

    bool DBusTorrent::createStream(uint file_index)
    {
        delete stream;

        stream = new DBusTorrentFileStream(file_index, this);
        if (!stream->ok())
        {
            delete stream;
            stream = 0;
            return false;
        }

        return true;
    }

    // streaming only works for one file at once atm, so the index has no effect yet
    bool DBusTorrent::removeStream(uint file_index)
    {
        Q_UNUSED(file_index);

        delete stream;
        stream = 0;
        return true;
    }

}

