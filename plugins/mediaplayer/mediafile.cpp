/*
    SPDX-FileCopyrightText: 2010 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mediafile.h"

#include <cmath>

#include <QFile>
#include <QMimeDatabase>
#include <QStringList>

#include "mediafilestream.h"
#include "mediaplayer.h"
#include <interfaces/torrentfileinterface.h>
#include <interfaces/torrentinterface.h>
#include <util/functions.h>

namespace kt
{
MediaFile::MediaFile(bt::TorrentInterface *tc)
    : tc(tc)
    , idx(INVALID_INDEX)
{
}

MediaFile::MediaFile(bt::TorrentInterface *tc, int idx)
    : tc(tc)
    , idx(idx)
{
}

MediaFile::MediaFile(const kt::MediaFile &mf)
    : tc(mf.tc)
    , idx(mf.idx)
{
}

MediaFile::~MediaFile()
{
}

QString MediaFile::name() const
{
    if (tc->getStats().multi_file_torrent) {
        if (idx < tc->getNumFiles()) {
            QString path = tc->getTorrentFile(idx).getUserModifiedPath();
            QVector<QStringRef> parts = path.splitRef(QLatin1Char('/'));
            if (parts.count() == 0)
                return path;
            else
                return parts.back().toString();
        } else
            return QString();
    } else {
        return tc->getDisplayName();
    }
}

QString MediaFile::path() const
{
    if (tc->getStats().multi_file_torrent) {
        if (idx < tc->getNumFiles())
            return tc->getTorrentFile(idx).getPathOnDisk();
        else
            return QString();
    } else {
        return tc->getStats().output_path;
    }
}

bool MediaFile::fullyAvailable() const
{
    if (tc->getStats().multi_file_torrent) {
        return idx < tc->getNumFiles() && std::fabs(tc->getTorrentFile(idx).getDownloadPercentage() - 100.0f) < 0.0001f;
    } else {
        return tc->getStats().completed;
    }
}

bool MediaFile::previewAvailable() const
{
    if (tc->getStats().multi_file_torrent) {
        return idx < tc->getNumFiles() && tc->getTorrentFile(idx).isPreviewAvailable();
    } else {
        return tc->readyForPreview();
    }
}

float MediaFile::downloadPercentage() const
{
    if (tc->getStats().multi_file_torrent) {
        if (idx < tc->getNumFiles())
            return tc->getTorrentFile(idx).getDownloadPercentage();
        else
            return 0.0f;
    } else {
        return bt::Percentage(tc->getStats());
    }
}

bt::Uint64 MediaFile::size() const
{
    if (tc->getStats().multi_file_torrent) {
        if (idx < tc->getNumFiles())
            return tc->getTorrentFile(idx).getSize();
        else
            return 0;
    } else {
        return tc->getStats().total_bytes;
    }
}

bt::Uint32 MediaFile::firstChunk() const
{
    if (tc->getStats().multi_file_torrent) {
        if (idx < tc->getNumFiles())
            return tc->getTorrentFile(idx).getFirstChunk();
        else
            return 0;
    } else {
        return 0;
    }
}

bt::Uint32 MediaFile::lastChunk() const
{
    if (tc->getStats().multi_file_torrent) {
        if (idx < tc->getNumFiles())
            return tc->getTorrentFile(idx).getLastChunk();
        else
            return 0;
    } else {
        return tc->getStats().total_chunks - 1;
    }
}

bt::TorrentFileStream::WPtr MediaFile::stream()
{
    if (!tfs) {
        // If some file is already in streaming mode, then try unstreamed mode
        tfs = tc->createTorrentFileStream(idx, true, 0);
        if (!tfs)
            tfs = tc->createTorrentFileStream(idx, false, 0);
    }

    return bt::TorrentFileStream::WPtr(tfs);
}

bool MediaFile::isVideo() const
{
    if (tc->getStats().multi_file_torrent) {
        return tc->getTorrentFile(idx).isVideo();
    } else {
        QMimeDatabase db;
        return db.mimeTypeForFile(path()).name().startsWith(QStringLiteral("video"));
    }
}

///////////////////////////////////////////////////////
MediaFileRef::MediaFileRef()
{
}

MediaFileRef::MediaFileRef(const QString &p)
    : file_path(p)
{
}

MediaFileRef::MediaFileRef(MediaFile::Ptr ptr)
    : ptr(ptr)
{
    file_path = ptr->path();
}

MediaFileRef::MediaFileRef(const kt::MediaFileRef &other)
    : ptr(other.ptr)
    , file_path(other.file_path)
{
}

MediaFileRef::~MediaFileRef()
{
}

MediaFileRef &MediaFileRef::operator=(const kt::MediaFileRef &other)
{
    ptr = other.ptr;
    file_path = other.file_path;
    return *this;
}

bool MediaFileRef::operator!=(const kt::MediaFileRef &other) const
{
    return file_path != other.path();
}

bool MediaFileRef::operator==(const kt::MediaFileRef &other) const
{
    return file_path == other.path();
}

Phonon::MediaSource MediaFileRef::createMediaSource(MediaPlayer *player)
{
    MediaFile::Ptr mf = mediaFile();
    if (mf && !mf->fullyAvailable()) {
        MediaFileStream *stream = new MediaFileStream(mf->stream());
        QObject::connect(stream, &MediaFileStream::stateChanged, player, &MediaPlayer::streamStateChanged);

        Phonon::MediaSource ms(stream);
        ms.setAutoDelete(true);
        return ms;
    } else
        return Phonon::MediaSource(QUrl::fromLocalFile(file_path));
}

QString MediaFileRef::name() const
{
    int idx = file_path.lastIndexOf(bt::DirSeparator());
    if (idx != -1)
        return file_path.mid(idx + 1);
    else
        return file_path;
}

}
