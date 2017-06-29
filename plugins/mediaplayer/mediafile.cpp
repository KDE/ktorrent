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

#include "mediafile.h"

#include <QFile>
#include <QMimeDatabase>
#include <QStringList>

#include <interfaces/torrentinterface.h>
#include <interfaces/torrentfileinterface.h>
#include <util/functions.h>
#include "mediafilestream.h"
#include "mediaplayer.h"


namespace kt
{
    MediaFile::MediaFile(bt::TorrentInterface* tc) : tc(tc), idx(INVALID_INDEX)
    {
    }

    MediaFile::MediaFile(bt::TorrentInterface* tc, int idx) : tc(tc), idx(idx)
    {
    }

    MediaFile::MediaFile(const kt::MediaFile& mf) : tc(mf.tc), idx(mf.idx)
    {
    }

    MediaFile::~MediaFile()
    {
    }

    QString MediaFile::name() const
    {
        if (tc->getStats().multi_file_torrent)
        {
            if (idx < tc->getNumFiles())
            {
                QString path = tc->getTorrentFile(idx).getUserModifiedPath();
                QVector<QStringRef> parts = path.splitRef(QLatin1Char('/'));
                if (parts.count() == 0)
                    return path;
                else
                    return parts.back().toString();
            }
            else
                return QString();
        }
        else
        {
            return tc->getDisplayName();
        }
    }


    QString MediaFile::path() const
    {
        if (tc->getStats().multi_file_torrent)
        {
            if (idx < tc->getNumFiles())
                return tc->getTorrentFile(idx).getPathOnDisk();
            else
                return QString();
        }
        else
        {
            return tc->getStats().output_path;
        }
    }

    bool MediaFile::fullyAvailable() const
    {
        if (tc->getStats().multi_file_torrent)
        {
            return idx < tc->getNumFiles() && qAbs(tc->getTorrentFile(idx).getDownloadPercentage() - 100.0f) < 0.0001f;
        }
        else
        {
            return tc->getStats().completed;
        }
    }

    bool MediaFile::previewAvailable() const
    {
        if (tc->getStats().multi_file_torrent)
        {
            return idx < tc->getNumFiles() && tc->getTorrentFile(idx).isPreviewAvailable();
        }
        else
        {
            return tc->readyForPreview();
        }
    }

    float MediaFile::downloadPercentage() const
    {
        if (tc->getStats().multi_file_torrent)
        {
            if (idx < tc->getNumFiles())
                return tc->getTorrentFile(idx).getDownloadPercentage();
            else
                return 0.0f;
        }
        else
        {
            return bt::Percentage(tc->getStats());
        }
    }

    bt::Uint64 MediaFile::size() const
    {
        if (tc->getStats().multi_file_torrent)
        {
            if (idx < tc->getNumFiles())
                return tc->getTorrentFile(idx).getSize();
            else
                return 0;
        }
        else
        {
            return tc->getStats().total_bytes;
        }
    }

    bt::Uint32 MediaFile::firstChunk() const
    {
        if (tc->getStats().multi_file_torrent)
        {
            if (idx < tc->getNumFiles())
                return tc->getTorrentFile(idx).getFirstChunk();
            else
                return 0;
        }
        else
        {
            return 0;
        }
    }

    bt::Uint32 MediaFile::lastChunk() const
    {
        if (tc->getStats().multi_file_torrent)
        {
            if (idx < tc->getNumFiles())
                return tc->getTorrentFile(idx).getLastChunk();
            else
                return 0;
        }
        else
        {
            return tc->getStats().total_chunks - 1;
        }
    }

    bt::TorrentFileStream::WPtr MediaFile::stream()
    {
        if (!tfs)
        {
            // If some file is already in streaming mode, then try unstreamed mode
            tfs = tc->createTorrentFileStream(idx, true, 0);
            if (!tfs)
                tfs = tc->createTorrentFileStream(idx, false, 0);
        }

        return bt::TorrentFileStream::WPtr(tfs);
    }

    bool MediaFile::isVideo() const
    {
        if (tc->getStats().multi_file_torrent)
        {
            return tc->getTorrentFile(idx).isVideo();
        }
        else
        {
            QMimeDatabase db;
            return db.mimeTypeForFile(path()).name().startsWith(QStringLiteral("video"));
        }
    }


    ///////////////////////////////////////////////////////
    MediaFileRef::MediaFileRef()
    {
    }

    MediaFileRef::MediaFileRef(const QString& p) : file_path(p)
    {
    }

    MediaFileRef::MediaFileRef(MediaFile::Ptr ptr) : ptr(ptr)
    {
        file_path = ptr->path();
    }

    MediaFileRef::MediaFileRef(const kt::MediaFileRef& other) : ptr(other.ptr), file_path(other.file_path)
    {
    }

    MediaFileRef::~MediaFileRef()
    {
    }

    MediaFileRef& MediaFileRef::operator=(const kt::MediaFileRef& other)
    {
        ptr = other.ptr;
        file_path = other.file_path;
        return *this;
    }

    bool MediaFileRef::operator!=(const kt::MediaFileRef& other) const
    {
        return file_path != other.path();
    }

    bool MediaFileRef::operator==(const kt::MediaFileRef& other) const
    {
        return file_path == other.path();
    }

    Phonon::MediaSource MediaFileRef::createMediaSource(MediaPlayer* player)
    {
        MediaFile::Ptr mf = mediaFile();
        if (mf && !mf->fullyAvailable())
        {
            MediaFileStream* stream = new MediaFileStream(mf->stream());
            QObject::connect(stream, SIGNAL(stateChanged(int)),
                             player, SLOT(streamStateChanged(int)));

            Phonon::MediaSource ms(stream);
            ms.setAutoDelete(true);
            return ms;
        }
        else
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

