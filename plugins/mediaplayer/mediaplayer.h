/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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

#ifndef KTAUDIOPLAYER_H
#define KTAUDIOPLAYER_H

#include <QStringList>
#include <QTimer>
#include <Phonon/MediaObject>
#include "mediafile.h"
#include "mediafilestream.h"


namespace Phonon
{
    class AudioOutput;
}


namespace kt
{
    enum ActionFlags
    {
        MEDIA_PLAY = 1, MEDIA_PAUSE = 2, MEDIA_STOP = 4, MEDIA_PREV = 8, MEDIA_NEXT = 16
    };

    /**
        @author
    */
    class MediaPlayer : public QObject
    {
        Q_OBJECT
    public:
        MediaPlayer(QObject* parent);
        ~MediaPlayer();

        Phonon::AudioOutput* output() {return audio;}
        Phonon::MediaObject* media0bject() {return media;}

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

    private slots:
        void onStateChanged(Phonon::State cur, Phonon::State old);
        void hasVideoChanged(bool hasVideo);
        void streamStateChanged(int state);

    signals:
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
        void playing(const MediaFileRef& file);

        /**
         * Emitted when the video is being loaded
         */
        void loading();

    private:
        Phonon::MediaObject* media;
        Phonon::AudioOutput* audio;
        QList<MediaFileRef> history;
        MediaFileRef current;
        bool buffering;
        bool manually_paused;
    };

}

#endif
