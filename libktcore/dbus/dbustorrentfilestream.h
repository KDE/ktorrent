/***************************************************************************
 *   Copyright (C) 2010 by Joris Guisson                                   *
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
        DBusTorrentFileStream(bt::Uint32 file_index, DBusTorrent* tor);
        ~DBusTorrentFileStream();

        /// Was the stream created properly ?
        bool ok() const {return !stream.isNull();}

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
        DBusTorrent* tor;
        bt::TorrentFileStream::Ptr stream;
    };

}

#endif // KT_DBUSTORRENTFILESTREAM_H
