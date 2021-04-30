/*
    SPDX-FileCopyrightText: 2010 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KT_DBUSTORRENTFILESTREAM_H
#define KT_DBUSTORRENTFILESTREAM_H

#include <QObject>
#include <torrent/torrentfilestream.h>

namespace kt
{
class DBusTorrent;

/**
 * DBus interface to a TorrentFileStream
 */
class DBusTorrentFileStream : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ktorrent.torrentfilestream")
public:
    DBusTorrentFileStream(bt::Uint32 file_index, DBusTorrent *tor);
    ~DBusTorrentFileStream() override;

    /// Was the stream created properly ?
    bool ok() const
    {
        return !stream.isNull();
    }

public Q_SLOTS:
    /// Get the current stream position
    Q_SCRIPTABLE qint64 pos() const;

    /// Get the total size
    Q_SCRIPTABLE qint64 size() const;

    /// Seek, will fail if attempting to seek to a point which is not downloaded yet
    Q_SCRIPTABLE bool seek(qint64 pos);

    /// How many bytes are there available
    Q_SCRIPTABLE qint64 bytesAvailable() const;

    /// Get the path of the file
    Q_SCRIPTABLE QString path() const;

    /// Get the current chunk relative to the first chunk of the file
    Q_SCRIPTABLE bt::Uint32 currentChunk() const;

    /// Read maxlen bytes from the stream
    Q_SCRIPTABLE QByteArray read(qint64 maxlen);

private:
    DBusTorrent *tor;
    bt::TorrentFileStream::Ptr stream;
};

}

#endif // KT_DBUSTORRENTFILESTREAM_H
