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

#ifndef KT_MEDIAFILESTREAM_H
#define KT_MEDIAFILESTREAM_H

#include <phonon/abstractmediastream.h>
#include <torrent/torrentfilestream.h>

namespace bt
{
    class TorrentFileStream;
}

namespace kt
{
    /**
        Class to stream a TorrentFileStream to phonon.
     */
    class MediaFileStream : public Phonon::AbstractMediaStream
    {
        Q_OBJECT
    public:
        MediaFileStream(bt::TorrentFileStream::WPtr stream, QObject* parent = 0);
        ~MediaFileStream();

        enum StreamState
        {
            PLAYING,
            BUFFERING
        };

        StreamState state() const {return waiting_for_data ? BUFFERING : PLAYING;}

    protected:
        void needData() override;
        void reset() override;
        void enoughData() override;
        void seekStream(qint64 offset) override;

    signals:
        /// Emitted when the stream state changes
        void stateChanged(int state);

    private slots:
        void dataReady();

    private:
        bt::TorrentFileStream::WPtr stream;
        bool waiting_for_data;
    };

}

#endif // KT_MEDIAFILESTREAM_H
