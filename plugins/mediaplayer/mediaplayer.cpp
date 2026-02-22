/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mediaplayer.h"

#include <QAudioOutput>

#include <KLocalizedString>

#include <torrent/torrentfilestream.h>
#include <util/log.h>

using namespace bt;

namespace kt
{
MediaPlayer::MediaPlayer(QObject *parent)
    : QObject(parent)
    , media(new QMediaPlayer(this))
    , audio(new QAudioOutput(this))
    , buffering(false)
    , manually_paused(false)
{
    media->setAudioOutput(audio);
    connect(media, &QMediaPlayer::mediaStatusChanged, this, &MediaPlayer::onMediaStatusChanged);
    connect(media, &QMediaPlayer::playbackStateChanged, this, &MediaPlayer::onPlaybackStateChanged);
    connect(media, &QMediaPlayer::hasVideoChanged, this, &MediaPlayer::hasVideoChanged);
}

bool MediaPlayer::paused() const
{
    return media->playbackState() == QMediaPlayer::PlaybackState::PausedState;
}

void MediaPlayer::resume()
{
    if (paused() || manually_paused) {
        if (buffering)
            manually_paused = false;
        else
            media->play();
    }
}

void MediaPlayer::play(kt::MediaFileRef file)
{
    buffering = false;
    Out(SYS_MPL | LOG_NOTICE) << "MediaPlayer: playing " << file.path() << endl;
    setMediaSource(file.createMediaSource());

    MediaFile::Ptr ptr = file.mediaFile();
    if (ptr && ptr->isVideo()) {
        Out(SYS_MPL | LOG_DEBUG) << "Opening video widget !" << endl;
        Q_EMIT openVideo();
    }

    history.append(file);
    Q_EMIT playing(file);
    media->play();
}

void MediaPlayer::pause()
{
    if (!buffering) {
        media->pause();
    } else {
        Out(SYS_MPL | LOG_DEBUG) << "MediaPlayer: paused" << endl;
        manually_paused = true;
        int flags = MEDIA_PLAY | MEDIA_STOP;
        if (history.count() > 1)
            flags |= MEDIA_PREV;

        Q_EMIT enableActions(flags);
    }
}

void MediaPlayer::stop()
{
    media->stop();
    if (buffering)
        buffering = false;

    current = MediaFileRef::MediaSource();
    onPlaybackStateChanged(QMediaPlayer::PlaybackState::StoppedState);
}

MediaFileRef MediaPlayer::prev()
{
    if (media->playbackState() == QMediaPlayer::PlaybackState::PausedState || media->playbackState() == QMediaPlayer::PlaybackState::PlayingState) {
        if (history.count() >= 2) {
            history.pop_back(); // remove the currently playing file
            MediaFileRef &file = history.back();
            setMediaSource(file.createMediaSource());
            media->play();
            Out(SYS_MPL | LOG_NOTICE) << "MediaPlayer: playing previous file " << file.path() << endl;
            return file;
        }
    } else {
        if (history.count() > 0) {
            MediaFileRef &file = history.back();
            setMediaSource(file.createMediaSource());
            media->play();
            Out(SYS_MPL | LOG_NOTICE) << "MediaPlayer: playing previous file " << file.path() << endl;
            return file;
        }
    }

    return QString();
}

void MediaPlayer::onMediaStatusChanged(QMediaPlayer::MediaStatus newState)
{
    unsigned int flags = 0;
    switch (newState) {
    case QMediaPlayer::MediaStatus::NoMedia:
        Out(SYS_MPL | LOG_DEBUG) << "MediaPlayer: no media" << endl;
        if (history.count() > 1) {
            flags |= MEDIA_PREV;
        }

        Q_EMIT enableActions(flags);
        break;
    case QMediaPlayer::MediaStatus::LoadingMedia:
        Out(SYS_MPL | LOG_DEBUG) << "MediaPlayer: loading" << endl;
        flags |= MEDIA_STOP;
        if (history.count() > 1)
            flags |= MEDIA_PREV;

        Q_EMIT enableActions(flags);
        Q_EMIT loading();
        break;
    case QMediaPlayer::MediaStatus::LoadedMedia:
        Out(SYS_MPL | LOG_DEBUG) << "MediaPlayer: loaded" << endl;
        flags = media->isPlaying() ? MEDIA_PAUSE : MEDIA_PLAY;
        flags |= MEDIA_STOP;
        if (history.count() > 1) {
            flags |= MEDIA_PREV;
        }

        Q_EMIT enableActions(flags);
        break;
    case QMediaPlayer::MediaStatus::StalledMedia:
        Out(SYS_MPL | LOG_DEBUG) << "MediaPlayer: stalled" << endl;
        break;
    case QMediaPlayer::MediaStatus::BufferingMedia:
        Out(SYS_MPL | LOG_DEBUG) << "MediaPlayer: buffering" << endl;
        flags = media->isPlaying() ? MEDIA_PAUSE : MEDIA_PLAY;
        flags |= MEDIA_STOP;
        if (history.count() > 1) {
            flags |= MEDIA_PREV;
        }

        Q_EMIT enableActions(flags);
        break;
    case QMediaPlayer::MediaStatus::BufferedMedia:
        Out(SYS_MPL | LOG_DEBUG) << "MediaPlayer: buffered" << endl;
        flags = media->isPlaying() ? MEDIA_PAUSE : MEDIA_PLAY;
        flags |= MEDIA_STOP;
        if (history.count() > 1) {
            flags |= MEDIA_PREV;
        }

        Q_EMIT enableActions(flags);
        break;
    case QMediaPlayer::MediaStatus::EndOfMedia:
        Out(SYS_MPL | LOG_DEBUG) << "MediaPlayer: end of media" << endl;
        Q_EMIT endOfMedia();
        if (history.count() > 1)
            flags |= MEDIA_PREV;

        Q_EMIT enableActions(flags);
        break;
    case QMediaPlayer::MediaStatus::InvalidMedia:
        Out(SYS_MPL | LOG_IMPORTANT) << "MediaPlayer: error " << media->errorString() << endl;
        flags = MEDIA_PLAY;
        if (history.count() > 1)
            flags |= MEDIA_PREV;

        Q_EMIT enableActions(flags);
        break;
    }
}

void MediaPlayer::onPlaybackStateChanged(QMediaPlayer::PlaybackState newState)
{
    unsigned int flags = 0;
    switch (newState) {
    case QMediaPlayer::PlaybackState::StoppedState:
        Out(SYS_MPL | LOG_DEBUG) << "MediaPlayer: stopped" << endl;
        flags = MEDIA_PLAY;
        if (history.count() > 1)
            flags |= MEDIA_PREV;

        Q_EMIT enableActions(flags);
        Q_EMIT stopped();
        break;
    case QMediaPlayer::PlaybackState::PlayingState:
        Out(SYS_MPL | LOG_DEBUG) << "MediaPlayer: playing " << getCurrentSource().path() << endl;
        flags = MEDIA_PAUSE | MEDIA_STOP;
        if (history.count() > 1)
            flags |= MEDIA_PREV;

        Q_EMIT enableActions(flags);
        hasVideoChanged(media->hasVideo());
        Q_EMIT playing(getCurrentSource());
        break;
    case QMediaPlayer::PlaybackState::PausedState:
        if (!buffering) {
            Out(SYS_MPL | LOG_DEBUG) << "MediaPlayer: paused" << endl;
            flags = MEDIA_PLAY | MEDIA_STOP;
            if (history.count() > 1)
                flags |= MEDIA_PREV;

            Q_EMIT enableActions(flags);
        }
        break;
    }
}

MediaFileRef MediaPlayer::getCurrentSource() const
{
    if (history.isEmpty())
        return MediaFileRef();
    else
        return MediaFileRef(history.back());
}

void MediaPlayer::hasVideoChanged(bool hasVideo)
{
    if (hasVideo)
        Q_EMIT openVideo();
    else
        Q_EMIT closeVideo();
}

void MediaPlayer::setMediaSource(const MediaFileRef::MediaSource &source)
{
    current = source;
    if (current.io_device) {
        media->setSourceDevice(current.io_device.get(), current.path);
    } else {
        media->setSource(current.path);
    }
}
}

#include "moc_mediaplayer.cpp"
