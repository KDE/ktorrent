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

#ifndef KT_MEDIAFILE_H
#define KT_MEDIAFILE_H

#include <QSharedPointer>
#include <QString>
#include <QWeakPointer>
#include <Phonon/MediaSource>

#include <util/constants.h>
#include <torrent/torrentfilestream.h>


class QObject;

namespace bt
{
    class TorrentInterface;
    class TorrentFileStream;
}

namespace kt
{

    class MediaPlayer;

    const bt::Uint32 INVALID_INDEX = 0xFFFFFFFF;

    /**
        Class representing a multimedia file of a torrent
    */
    class MediaFile
    {
    public:
        /**
            Constructor for single file torrents
            @param tc The TorrentInterface
        */
        MediaFile(bt::TorrentInterface* tc);

        /**
            Constructor for multi file torrents
            @param tc The TorrentInterface
            @param idx The index of the file in the torrent
        */
        MediaFile(bt::TorrentInterface* tc, int idx);

        /**
            Copy constructor
            @param mf The MediaFile to copy
        */
        MediaFile(const MediaFile& mf);
        ~MediaFile();

        /// Get the path of the MediaFile
        QString path() const;

        /// Get the name of the MediaFile
        QString name() const;

        /// Is a preview available
        bool previewAvailable() const;

        /// Is it fully available
        bool fullyAvailable() const;

        /// Get the download percentage
        float downloadPercentage() const;

        /// Get the torrent of this MediaFile
        bt::TorrentInterface* torrent() const {return tc;}

        /// Get the size of the MediaFile
        bt::Uint64 size() const;

        /// Get the first chunk of the file
        bt::Uint32 firstChunk() const;

        /// Get the last chunk of the file
        bt::Uint32 lastChunk() const;

        /// Create a TorrentFileStream object for this MediaFile and return a weak pointer to it
        bt::TorrentFileStream::WPtr stream();

        /// Is this a video ?
        bool isVideo() const;

        typedef QSharedPointer<MediaFile> Ptr;
        typedef QWeakPointer<MediaFile> WPtr;
    private:
        bt::TorrentInterface* tc;
        bt::Uint32 idx;
        bt::TorrentFileStream::Ptr tfs;
    };

    /**
        A MediaFileRef is a reference to a MediaFile
        which keeps a weak pointer and the path of the file as backup.
        If the weak pointer can no longer resolve to a strong pointer
        the actual path can still be used.

        This can also be used for files only (without an actual MediaFile)
    */
    class MediaFileRef
    {
    public:
        /// Default constructor
        MediaFileRef();

        /// Simple file mode constructor
        MediaFileRef(const QString& p);

        /// Strong pointer constructor
        MediaFileRef(MediaFile::Ptr ptr);

        /// Copy constructor
        MediaFileRef(const MediaFileRef& other);
        ~MediaFileRef();

        /// Get the MediaFile
        MediaFile::Ptr mediaFile() {return ptr.toStrongRef();}

        /// Get the path
        QString path() const {return file_path;}

        /// Get the name of the file
        QString name() const;

        /// Assignment operator
        MediaFileRef& operator = (const MediaFileRef& other);

        /// Comparison operator
        bool operator == (const MediaFileRef& other) const;

        /// Negative comparison operator
        bool operator != (const MediaFileRef& other) const;

        /// Create a Phonon::MediaSource for this MediaFileRef
        Phonon::MediaSource createMediaSource(MediaPlayer* p);

    private:
        MediaFile::WPtr ptr;
        QString file_path;
    };

}

#endif // KT_MEDIAFILE_H
