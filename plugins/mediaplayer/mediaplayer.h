/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTAUDIOPLAYER_H
#define KTAUDIOPLAYER_H

#include "mediafile.h"
#include <QMediaPlayer>
#include <QStringList>
#include <QTimer>

class QAudioOutput;

namespace kt
{
enum ActionFlags {
    MEDIA_PLAY = 1,
    MEDIA_PAUSE = 2,
    MEDIA_STOP = 4,
    MEDIA_PREV = 8,
    MEDIA_NEXT = 16,
};

/**
    @author
*/
class MediaPlayer : public QObject
{
    Q_OBJECT
public:
    MediaPlayer(QObject *parent);
    ~MediaPlayer() override = default;

    QMediaPlayer *mediaPlayer()
    {
        return media;
    }

    /// Are we paused
    bool paused() const;

    /// Resume paused stuff
    void resume();

    /// Play a file
    void play(MediaFileRef file);

    /// Pause playing
    void pause();

    /// Stop playing
    void stop();

    /// Get the current file we are playing
    MediaFileRef getCurrentSource() const;

    /// Play the previous song
    MediaFileRef prev();

    /// Whether the current media source is a torrent stream
    bool isTorrentSource() const
    {
        return current.io_device != nullptr;
    }

private Q_SLOTS:
    void onMediaStatusChanged(QMediaPlayer::MediaStatus newState);
    void onPlaybackStateChanged(QMediaPlayer::PlaybackState newState);
    void hasVideoChanged(bool hasVideo);

Q_SIGNALS:
    /**
     * Emitted to enable or disable the play buttons.
     * @param flags Flags indicating which buttons to enable
     */
    void enableActions(unsigned int flags);

    /**
     * A video has been detected, create the video player window.
     */
    void openVideo();

    /**
     * Emitted when the video widget needs to be closed.
     */
    void closeVideo();

    /**
     * Emitted when we have finished playing something
     */
    void stopped();

    /**
     * Emitted when the player has reached the end of the current media
     */
    void endOfMedia();

    /**
     * Emitted when the player starts playing
     */
    void playing(const MediaFileRef &file);

    /**
     * Emitted when the video is being loaded
     */
    void loading();

private:
    void setMediaSource(const MediaFileRef::MediaSource &source);

    QMediaPlayer *media;
    QAudioOutput *audio;
    QList<MediaFileRef> history;
    MediaFileRef::MediaSource current;
    MediaFileRef next;
    bool buffering;
    bool manually_paused;
    bool end_of_media = false;
};

}

#endif
