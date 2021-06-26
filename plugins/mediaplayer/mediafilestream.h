/*
    SPDX-FileCopyrightText: 2010 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
    MediaFileStream(bt::TorrentFileStream::WPtr stream, QObject *parent = nullptr);
    ~MediaFileStream() override;

    enum StreamState {
        PLAYING,
        BUFFERING,
    };

    StreamState state() const
    {
        return waiting_for_data ? BUFFERING : PLAYING;
    }

protected:
    void needData() override;
    void reset() override;
    void enoughData() override;
    void seekStream(qint64 offset) override;

Q_SIGNALS:
    /// Emitted when the stream state changes
    void stateChanged(int state);

private Q_SLOTS:
    void dataReady();

private:
    bt::TorrentFileStream::WPtr stream;
    bool waiting_for_data;
};

}

#endif // KT_MEDIAFILESTREAM_H
