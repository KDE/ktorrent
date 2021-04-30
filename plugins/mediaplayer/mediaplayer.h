/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTAUDIOPLAYER_H
#define KTAUDIOPLAYER_H

#include "mediafile.h"
#include "mediafilestream.h"
#include <Phonon/MediaObject>
#include <QStringList>
#include <QTimer>

namespace Phonon
{
class AudioOutput;
}

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
    ~MediaPlayer() override;

    Phonon::AudioOutput *output()
    {
        return audio;
    }
    Phonon::MediaObject *media0bject()
    {
        return media;
    }

    /// Are we paused
    bool paused() const;

    /// Resume paused stuff
    void resume();

    /// Play a file
    void play(MediaFileRef file);

    /// Queue a file
    void queue(MediaFileRef file);

    /// Pause playing
    void pause();

    /// Stop playing
    void stop();

    /// Get the current file we are playing
    MediaFileRef getCurrentSource() const;

    /// Play the previous song
    MediaFileRef prev();

    void streamStateChanged(int state);

private Q_SLOTS:
    void onStateChanged(Phonon::State cur, Phonon::State old);
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
     * Emitted when the player is about to finish
     */
    void aboutToFinish();

    /**
     * Emitted when the player starts playing
     */
    void playing(const MediaFileRef &file);

    /**
     * Emitted when the video is being loaded
     */
    void loading();

private:
    Phonon::MediaObject *media;
    Phonon::AudioOutput *audio;
    QList<MediaFileRef> history;
    MediaFileRef current;
    bool buffering;
    bool manually_paused;
};

}

#endif
