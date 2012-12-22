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

#include "mediafilestream.h"
#include <torrent/torrentfilestream.h>
#include <util/log.h>
#include <settings.h>

using namespace bt;

namespace kt
{
    const Uint32 MIN_AMOUNT_NEEDED = 16 * 1024;


    MediaFileStream::MediaFileStream(bt::TorrentFileStream::WPtr stream, QObject* parent)
        : AbstractMediaStream(parent), stream(stream), waiting_for_data(false)
    {
        TorrentFileStream::Ptr s = stream.toStrongRef();
        if (s)
        {
            s->open(QIODevice::ReadOnly);
            s->reset();
            setStreamSize(s->size());
            setStreamSeekable(!s->isSequential());
            connect(s.data(), SIGNAL(readyRead()), this, SLOT(dataReady()));
        }
    }

    MediaFileStream::~MediaFileStream()
    {
    }

    void MediaFileStream::dataReady()
    {
        if (waiting_for_data)
        {
            TorrentFileStream::Ptr s = stream.toStrongRef();
            // Make sure there is enough data buffered for smooth playback
            if (s)
            {
                qint64 left = s->size() - s->pos();
                qint64 min_amount_needed = MIN_AMOUNT_NEEDED;
                if (left < min_amount_needed)
                    min_amount_needed = left;

                if (s->bytesAvailable() >= min_amount_needed)
                {
                    const QByteArray data = s->read(min_amount_needed);
                    if (!data.isEmpty())
                    {
                        writeData(data);
                        waiting_for_data = false;
                        stateChanged(PLAYING);
                    }
                }
                else
                {
                    Out(SYS_MPL | LOG_DEBUG) << "Not enough data available: " << s->bytesAvailable()  << " (need " << min_amount_needed <<  ")" << endl;
                    stateChanged(BUFFERING);
                }
            }
            else
                endOfData();
        }
    }

    void MediaFileStream::needData()
    {
        //  Out(SYS_GEN|LOG_DEBUG) << "MediaFileStream::needData" << endl;
        TorrentFileStream::Ptr s = stream.toStrongRef();
        if (!s || s->atEnd())
        {
            endOfData();
            return;
        }

        // Make sure there is enough data buffered for smooth playback
        qint64 left = s->size() - s->pos();
        qint64 min_amount_needed = MIN_AMOUNT_NEEDED;
        if (left < min_amount_needed)
            min_amount_needed = left;

        if (s->bytesAvailable() >= min_amount_needed)
        {
            QByteArray data = s->read(min_amount_needed);
            if (data.isEmpty())
            {
                waiting_for_data = true;
            }
            else
            {
                writeData(data);
                if (waiting_for_data)
                {
                    waiting_for_data = false;
                    stateChanged(PLAYING);
                }
            }
        }
        else
        {
            Out(SYS_MPL | LOG_DEBUG) << "Not enough data available: " << s->bytesAvailable()  << " (need " << min_amount_needed <<  ")" << endl;
            waiting_for_data = true;
            stateChanged(BUFFERING);

            // Send some more data, otherwise phonon seems to get stuck
            QByteArray data = s->read(4096);
            if (!data.isEmpty())
                writeData(data);
        }
    }

    void MediaFileStream::reset()
    {
        TorrentFileStream::Ptr s = stream.toStrongRef();
        if (s)
            s->reset();
    }

    void MediaFileStream::enoughData()
    {
        //  Out(SYS_GEN|LOG_DEBUG) << "MediaFileStream::enoughData" << endl;
        //  waiting_for_data = false;
    }

    void MediaFileStream::seekStream(qint64 offset)
    {
        TorrentFileStream::Ptr s = stream.toStrongRef();
        if (!s)
            return;

        s->seek(offset);
    }
}
