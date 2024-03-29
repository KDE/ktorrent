/*
    SPDX-FileCopyrightText: 2010 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "dbustorrentfilestream.h"
#include "dbustorrent.h"

#include <algorithm>

#include <QDBusConnection>
#include <util/sha1hash.h>

namespace kt
{
DBusTorrentFileStream::DBusTorrentFileStream(bt::Uint32 file_index, kt::DBusTorrent *tor)
    : QObject(tor)
    , tor(tor)
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

    qint64 to_read = std::min(maxlen, bytesAvailable());
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

#include "moc_dbustorrentfilestream.cpp"
