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

#ifndef KTVIDEOWIDGET_H
#define KTVIDEOWIDGET_H

#include <QAction>
#include <QWidget>
#include <Phonon/VideoWidget>
#include <Phonon/MediaObject>
#include <Phonon/SeekSlider>
#include <Phonon/VolumeSlider>

class QAction;
class QDBusInterface;
class QLabel;
class QStackedWidget;
class KToolBar;
class KActionCollection;

namespace kt
{

    class VideoChunkBar;
    class MediaPlayer;
    class MediaFileRef;

    /**
     * Widget to display a video
     * @author Joris Guisson
    */
    class VideoWidget : public QWidget
    {
        Q_OBJECT
    public:
        VideoWidget(MediaPlayer* player, KActionCollection* ac, QWidget* parent);
        ~VideoWidget();

        /**
         * Make the widget full screen or not.
         * @param on
         */
        void setFullScreen(bool on);

    protected:
        void mouseMoveEvent(QMouseEvent* event) override;
        bool eventFilter(QObject* dst, QEvent* event) override;

    private slots:
        void play();
        void stop();
        void setControlsVisible(bool on);
        void timerTick(qint64 time);
        void playing(const MediaFileRef& mfile);
        void enableActions(unsigned int flags);

    signals:
        void toggleFullScreen(bool on);

    private:
        void inhibitScreenSaver(bool on);
        QString formatTime(qint64 cur, qint64 total);

    private:
        Phonon::VideoWidget* video;
        MediaPlayer* player;
        Phonon::SeekSlider* slider;
        KToolBar* tb;
        QAction * play_action;
        QAction * stop_action;
        QLabel* time_label;
        Phonon::VolumeSlider* volume;
        VideoChunkBar* chunk_bar;
        bool fullscreen;
        quint32 screensaver_cookie;
        quint32 powermanagement_cookie;
    };

}

#endif
