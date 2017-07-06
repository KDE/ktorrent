/***************************************************************************
 *   Copyright (C) 2014 by Juan Palacios                                   *
 *   jpalaciosdev@gmail.com                                                *
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

#ifndef MAGNETMANAGER_H
#define MAGNETMANAGER_H

#include <interfaces/coreinterface.h>
#include <magnet/magnetdownloader.h>
#include <bcodec/bencoder.h>

namespace kt {

/// Adds options struct to bt::MagnetDownloader
class MagnetDownloader : public bt::MagnetDownloader
{
    Q_OBJECT
public:
    MagnetDownloader(const bt::MagnetLink& mlink, const MagnetLinkLoadOptions& options, QObject* parent)
        : bt::MagnetDownloader(mlink, parent)
        , options(options) {}
    ~MagnetDownloader() {}

    MagnetLinkLoadOptions options;
};

/// This class represent a downloading slot.
/// A downloading slot has the index of the magnet that occupy it and a timer that
/// controls the maximum time that one magnet can occupy the downloading slot.
class DownloadSlot : public QObject
{
    Q_OBJECT
public:
    DownloadSlot(QObject *parent = nullptr);
    ~DownloadSlot();

    void setTimerDuration(unsigned int duration);
    void startTimer();
    void stopTimer();
    void reset();
    void setMagnetIndex(int index);
    int getMagnetIndex() const;
    bool isTimerActived() const;
    bool isOccupied() const;

signals:
    void timeout(int magnetIdx);

private slots:
    void onTimeout();

private:
    int magnetIdx;
    unsigned int timerDuration;
    QTimer* timer;
};

/// This class manage the downloading of magnets.
/// For this task uses a queue with a determined number of concurrent downloadings.
/// All downloading magnets uses a timer that determines the maximum time
/// that each magnet can be in the downloading state. If the magnet is not downloaded
/// within this time, that magnet will be pushed back at the end of the queued list,
/// just above the stopped magnets list.
/// The stopped magnet links always will occupy the latests positions of the queue.
class KTCORE_EXPORT MagnetManager : public QObject
{
    Q_OBJECT
public:
    MagnetManager(QObject* parent = nullptr);
    ~MagnetManager();

    /// Adds a magnet link to the queue
    /// @param mlink magnet link to be added
    /// @param options magnet link options
    /// @param stopped whether this magnet should be added to the queue stopped
    void addMagnet(const bt::MagnetLink& mlink, const MagnetLinkLoadOptions& options, bool stopped);

    /// Removes count successive magnets, starting on idx
    void removeMagnets(bt::Uint32 idx, bt::Uint32 count);

    /// Starts count successive magnets, starting on idx
    void start(bt::Uint32 idx, bt::Uint32 count);

    /// Stops count successive magnets, starting on idx
    void stop(bt::Uint32 idx, bt::Uint32 count);

    /// Returns whether the magnet corresponding with idx is stopped
    bool isStopped(bt::Uint32 idx) const;

    /// Set the number of concurrent downloading magnets
    void setDownloadingSlots(bt::Uint32 count);

    /// Sets if the slot timer must be used
    void setUseSlotTimer(bool value);

    /// Set the maximum time that a magnet link is in downloading state
    /// @param duration time in minutes
    void setTimerDuration(bt::Uint32 duration);

    /// Updates the downloading magnets
    void update();

    /// Load all magnets from a file
    void loadMagnets(const QString& file);

    /// Save all magnets to a file
    void saveMagnets(const QString& file);

    /// Defines the magnet state on the MagnetManager
    enum MagnetState
    {
        DOWNLOADING,    ///< Started and downloading
        QUEUED,         ///< Started and waiting for download
        STOPPED         ///< Stopped
    };

    /// Return the state of the magnet corresponding to idx
    MagnetState status(bt::Uint32 idx) const;

    /// Return the number of managed magnets
    int count() const;

    /// Get the magnet downloader at index idx in the list
    /// @param idx index of the magnet
    /// @return the magnet downloader or nullptr if idx is out of bounds
    const kt::MagnetDownloader* getMagnetDownloader(bt::Uint32 idx) const;

signals:
    /// Emitted when metadata has been downloaded for a MagnetLink.
    void metadataDownloaded(const bt::MagnetLink& mlink, const QByteArray& data, const kt::MagnetLinkLoadOptions& options);

    /// Emitted when the queue has been altered in some form, so the
    /// magnets order and/or number could be altered.
    /// The range of the altered magnets is determined by idx and count.
    /// @param idx determines the index of the first altered magnet
    /// @param count determines the number of magnets that must be updated
    void updateQueue(bt::Uint32 idx, bt::Uint32 count);

private slots:
    void onDownloadFinished(bt::MagnetDownloader* md, const QByteArray& data);
    void onSlotTimeout(int magnetIdx);

private:
    /// Start the next queued magnets and return the index of the first started magnet
    int startNextQueuedMagnets();

    /// Free the download slot that is occuping the magnet with magnetIdx, updating
    /// the indices of the magnets in all magnets slots to keep it in sync with the magnet
    /// queue indices.
    void freeDownloadSlot(bt::Uint32 magnetIdx);

    /// Return the magnet index in the queue + the stopped list
    int getMagnetIndex(kt::MagnetDownloader* md);

    /// Writes the encoder info of one magnet
    void writeEncoderInfo(bt::BEncoder &enc, kt::MagnetDownloader* md);

    bool useSlotTimer;
    int timerDuration;
    QList<DownloadSlot*> usedDownloadingSlots;
    QList<DownloadSlot*> freeDownloadingSlots;
    QList<kt::MagnetDownloader*> magnetQueue;
    QList<kt::MagnetDownloader*> stoppedList;
    QSet<bt::SHA1Hash> magnetHashes;
    QSet<bt::SHA1Hash> stoppedHashes;
};

}

#endif // MAGNETMANAGER_H
