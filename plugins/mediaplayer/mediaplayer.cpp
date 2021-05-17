/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <KLocalizedString>
#include <Phonon/AudioOutput>
#include <Phonon/Global>
#include <Phonon/Path>

#include "mediaplayer.h"
#include <torrent/torrentfilestream.h>
#include <util/log.h>

using namespace bt;

namespace kt
{
MediaPlayer::MediaPlayer(QObject *parent)
    : QObject(parent)
    , buffering(false)
    , manually_paused(false)
{
    media = new Phonon::MediaObject(this);
    audio = new Phonon::AudioOutput(this);
    Phonon::createPath(media, audio);

    connect(media, &Phonon::MediaObject::stateChanged, this, &MediaPlayer::onStateChanged);
    connect(media, &Phonon::MediaObject::hasVideoChanged, this, &MediaPlayer::hasVideoChanged);
    connect(media, &Phonon::MediaObject::aboutToFinish, this, &MediaPlayer::aboutToFinish);
    media->setTickInterval(1000);
}

MediaPlayer::~MediaPlayer()
{
    stop();
}

bool MediaPlayer::paused() const
{
    return media->state() == Phonon::PausedState;
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
    Phonon::MediaSource ms = file.createMediaSource(this);
    media->setCurrentSource(ms);

    MediaFile::Ptr ptr = file.mediaFile();
    if (ptr && ptr->isVideo()) {
        Out(SYS_MPL | LOG_DEBUG) << "Opening video widget !" << endl;
        openVideo();
    }

    history.append(file);
    playing(file);
    current = file;
    media->play();
}

void MediaPlayer::queue(kt::MediaFileRef file)
{
    Out(SYS_MPL | LOG_NOTICE) << "MediaPlayer: enqueue " << file.path() << endl;
    media->enqueue(file.createMediaSource(this));
    history.append(file);
    onStateChanged(media->state(), Phonon::StoppedState);
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

        enableActions(flags);
    }
}

void MediaPlayer::stop()
{
    media->stop();
    media->clear();
    if (buffering)
        buffering = false;

    current = MediaFileRef();
    onStateChanged(media->state(), Phonon::StoppedState);
}

MediaFileRef MediaPlayer::prev()
{
    if (media->state() == Phonon::PausedState || media->state() == Phonon::PlayingState) {
        if (history.count() >= 2) {
            history.pop_back(); // remove the currently playing file
            MediaFileRef &file = history.back();
            media->setCurrentSource(file.createMediaSource(this));
            media->play();
            Out(SYS_MPL | LOG_NOTICE) << "MediaPlayer: playing previous file " << file.path() << endl;
            return file;
        }
    } else {
        if (history.count() > 0) {
            MediaFileRef &file = history.back();
            media->setCurrentSource(file.createMediaSource(this));
            media->play();
            Out(SYS_MPL | LOG_NOTICE) << "MediaPlayer: playing previous file " << file.path() << endl;
            return file;
        }
    }

    return QString();
}

void MediaPlayer::onStateChanged(Phonon::State cur, Phonon::State)
{
    unsigned int flags = 0;
    switch (cur) {
    case Phonon::LoadingState:
        Out(SYS_MPL | LOG_DEBUG) << "MediaPlayer: loading" << endl;
        if (history.count() > 0)
            flags |= MEDIA_PREV;

        enableActions(flags);
        loading();
        break;
    case Phonon::StoppedState:
        Out(SYS_MPL | LOG_DEBUG) << "MediaPlayer: stopped" << endl;
        flags = MEDIA_PLAY;
        if (history.count() > 0)
            flags |= MEDIA_PREV;

        enableActions(flags);
        stopped();
        break;
    case Phonon::PlayingState:
        Out(SYS_MPL | LOG_DEBUG) << "MediaPlayer: playing " << getCurrentSource().path() << endl;
        flags = MEDIA_PAUSE | MEDIA_STOP;
        if (history.count() > 1)
            flags |= MEDIA_PREV;

        enableActions(flags);
        hasVideoChanged(media->hasVideo());
        playing(getCurrentSource());
        break;
    case Phonon::BufferingState:
        Out(SYS_MPL | LOG_DEBUG) << "MediaPlayer: buffering" << endl;
        break;
    case Phonon::PausedState:
        if (!buffering) {
            Out(SYS_MPL | LOG_DEBUG) << "MediaPlayer: paused" << endl;
            flags = MEDIA_PLAY | MEDIA_STOP;
            if (history.count() > 1)
                flags |= MEDIA_PREV;

            enableActions(flags);
        }
        break;
    case Phonon::ErrorState:
        Out(SYS_MPL | LOG_IMPORTANT) << "MediaPlayer: error " << media->errorString() << endl;
        flags = MEDIA_PLAY;
        if (history.count() > 0)
            flags |= MEDIA_PREV;

        enableActions(flags);
        break;
    }
}

void MediaPlayer::streamStateChanged(int state)
{
    Out(SYS_MPL | LOG_DEBUG) << "Stream state changed: " << (state == MediaFileStream::BUFFERING ? "BUFFERING" : "PLAYING") << endl;
    if (state == MediaFileStream::BUFFERING) {
        buffering = true;
        media->pause();
        onStateChanged(media->state(), Phonon::PlayingState);
    } else if (buffering) {
        buffering = false;
        if (!manually_paused)
            media->play();
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
        openVideo();
    else
        closeVideo();
}

}
