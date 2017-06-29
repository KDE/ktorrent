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

#include "dbustorrentfilestream.h"
#include "dbustorrent.h"

#include <QDBusConnection>
#include <util/sha1hash.h>

namespace kt
{

    DBusTorrentFileStream::DBusTorrentFileStream(bt::Uint32 file_index, kt::DBusTorrent* tor) : QObject(tor), tor(tor)
    {
        QDBusConnection sb = QDBusConnection::sessionBus();
        QString path = QStringLiteral("/torrent/%1/stream").arg(tor->torrent()->getInfoHash().toString());
        QFlags<QDBusConnection::RegisterOption> flags = QDBusConnection::ExportScriptableSlots | QDBusConnection::ExportScriptableSignals;
        sb.registerObject(path, this, flags);

        stream = tor->torrent()->createTorrentFileStream(file_index, true, this);
        if (stream)
            stream->open(QIODevice::ReadOnly);
    }

    DBusTorrentFileStream::~DBusTorrentFileStream()
    {
    }

    qint64 DBusTorrentFileStream::bytesAvailable() const
    {
        return stream ? stream->bytesAvailable() : 0;
    }

    bt::Uint32 DBusTorrentFileStream::currentChunk() const
    {
        return stream ? stream->currentChunk() : 0;
    }

    QString DBusTorrentFileStream::path() const
    {
        return stream ? stream->path() : QString();
    }

    qint64 DBusTorrentFileStream::pos() const
    {
        return stream ? stream->pos() : 0;
    }

    QByteArray DBusTorrentFileStream::read(qint64 maxlen)
    {
        if (!stream || bytesAvailable() == 0)
            return QByteArray();

        qint64 to_read = qMin(maxlen, bytesAvailable());
        QByteArray ba(to_read, 0);

        qint64 ret = stream->read(ba.data(), to_read);
        if (ret < to_read)
            ba.resize(ret);
        return ba;
    }

    bool DBusTorrentFileStream::seek(qint64 pos)
    {
        return stream ? stream->seek(pos) : false;
    }

    qint64 DBusTorrentFileStream::size() const
    {
        return stream ? stream->size() : 0;
    }

}

