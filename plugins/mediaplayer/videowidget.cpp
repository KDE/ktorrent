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
#include "videowidget.h"

#include <QAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QStackedWidget>
#include <QIcon>
#include <klocalizedstring.h>
#include <KToolBar>
#include <KToggleFullScreenAction>
#include <KActionCollection>
#include <Phonon/Path>
#include <Phonon/AudioOutput>
#include <Phonon/Global>
#include <Phonon/SeekSlider>
#include <Phonon/VolumeSlider>
#include <solid/powermanagement.h>
#include <util/log.h>
#include <torrent/chunkbar.h>
#include "mediaplayer.h"
#include "videochunkbar.h"
#include "screensaver_interface.h"


using namespace bt;

namespace kt
{


    VideoWidget::VideoWidget(MediaPlayer* player, KActionCollection* ac, QWidget* parent)
        : QWidget(parent), player(player), chunk_bar(0), fullscreen(false),
          screensaver_cookie(0), powermanagement_cookie(0)
    {
        QVBoxLayout* vlayout = new QVBoxLayout(this);
        vlayout->setMargin(0);
        vlayout->setSpacing(0);

        video = new Phonon::VideoWidget(this);
        Phonon::createPath(player->media0bject(), video);
        video->installEventFilter(this);

        chunk_bar = new VideoChunkBar(player->getCurrentSource(), this);
        chunk_bar->setVisible(player->media0bject()->currentSource().type() == Phonon::MediaSource::Stream);

        QHBoxLayout* hlayout = new QHBoxLayout(0);

        play_action = new QAction(QIcon::fromTheme("media-playback-start"), i18n("Play"), this);
        connect(play_action, SIGNAL(triggered()), this, SLOT(play()));

        stop_action = new QAction(QIcon::fromTheme("media-playback-stop"), i18n("Stop"), this);
        connect(stop_action, SIGNAL(triggered()), this, SLOT(stop()));

        tb = new KToolBar(this);
        tb->setToolButtonStyle(Qt::ToolButtonIconOnly);
        tb->addAction(play_action);
        tb->addAction(ac->action("media_pause"));
        tb->addAction(stop_action);
        QAction* tfs = ac->action("video_fullscreen");
        connect(tfs, SIGNAL(toggled(bool)), this, SIGNAL(toggleFullScreen(bool)));
        tb->addAction(tfs);

        slider = new Phonon::SeekSlider(this);
        slider->setMediaObject(player->media0bject());
        slider->setMaximumHeight(tb->iconSize().height());

        volume = new Phonon::VolumeSlider(this);
        volume->setAudioOutput(player->output());
        volume->setMaximumHeight(tb->iconSize().height());
        volume->setMaximumWidth(5 * tb->iconSize().width());

        time_label = new QLabel(this);
        time_label->setText(formatTime(player->media0bject()->currentTime(), player->media0bject()->totalTime()));
        time_label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

        hlayout->addWidget(tb);
        hlayout->addWidget(slider);
        hlayout->addWidget(volume);
        hlayout->addWidget(time_label);

        chunk_bar->setFixedHeight(hlayout->sizeHint().height() * 0.75);

        vlayout->addWidget(chunk_bar);
        vlayout->addWidget(video);
        vlayout->addLayout(hlayout);

        connect(player->media0bject(), SIGNAL(tick(qint64)), this, SLOT(timerTick(qint64)));
        connect(player, SIGNAL(playing(MediaFileRef)), this, SLOT(playing(MediaFileRef)));
        connect(player, SIGNAL(enableActions(unsigned int)), this, SLOT(enableActions(unsigned int)));
        inhibitScreenSaver(true);
    }


    VideoWidget::~VideoWidget()
    {
        inhibitScreenSaver(false);
    }

    void VideoWidget::play()
    {
        player->media0bject()->play();
    }

    void VideoWidget::stop()
    {
        Phonon::MediaObject* mo = player->media0bject();
        if (mo->state() == Phonon::PausedState)
        {
            mo->seek(0);
            mo->stop();
        }
        else
        {
            mo->stop();
        }
    }


    void VideoWidget::setControlsVisible(bool on)
    {
        slider->setVisible(on);
        volume->setVisible(on);
        tb->setVisible(on);
        chunk_bar->setVisible(player->media0bject()->currentSource().type() == Phonon::MediaSource::Stream && on);
        time_label->setVisible(on);
    }

    bool VideoWidget::eventFilter(QObject* dst, QEvent* event)
    {
        Q_UNUSED(dst);
        if (fullscreen && event->type() == QEvent::MouseMove)
            mouseMoveEvent((QMouseEvent*)event);

        return true;
    }


    void VideoWidget::mouseMoveEvent(QMouseEvent* event)
    {
        if (!fullscreen)
            return;

        bool streaming = player->media0bject()->currentSource().type() == Phonon::MediaSource::Stream;
        if (slider->isVisible())
        {
            int bh = height() - slider->height();
            int th = streaming ? chunk_bar->height() : 0;
            if (event->y() < bh - 10 && event->y() > th + 10) // use a 10 pixel safety buffer to avoid fibrilation
                setControlsVisible(false);
        }
        else
        {
            int bh = height() - slider->height();
            int th = streaming ? chunk_bar->height() : 0;
            if (event->y() >= bh || event->y() <= th)
                setControlsVisible(true);
        }
    }

    void VideoWidget::setFullScreen(bool on)
    {
        if (on)
        {
            setWindowState(windowState() | Qt::WindowFullScreen);
            setControlsVisible(false);
        }
        else
        {
            setWindowState(windowState() & ~Qt::WindowFullScreen);
            setControlsVisible(true);
        }
        fullscreen = on;
        setMouseTracking(fullscreen);
    }

    void VideoWidget::inhibitScreenSaver(bool on)
    {
        QString interface("org.freedesktop.ScreenSaver");
        org::freedesktop::ScreenSaver screensaver(interface, "/ScreenSaver", QDBusConnection::sessionBus());
        if (on)
        {
            QString msg = i18n("KTorrent is playing a video.");
            screensaver_cookie = screensaver.Inhibit("ktorrent", msg);
            Out(SYS_MPL | LOG_NOTICE) << "Screensaver inhibited (cookie " << screensaver_cookie << ")" << endl;
            powermanagement_cookie = Solid::PowerManagement::beginSuppressingSleep(msg);
            Out(SYS_MPL | LOG_NOTICE) << "PowerManagement inhibited (cookie " << powermanagement_cookie << ")" << endl;
        }
        else
        {
            screensaver.UnInhibit(screensaver_cookie);
            powermanagement_cookie = Solid::PowerManagement::stopSuppressingSleep(powermanagement_cookie);
            Out(SYS_MPL | LOG_NOTICE) << "Screensaver uninhibited" << endl;
            Out(SYS_MPL | LOG_NOTICE) << "PowerManagement uninhibited" << endl;
        }
    }

    void VideoWidget::timerTick(qint64 time)
    {
        time_label->setText(formatTime(time, player->media0bject()->totalTime()));
        if (chunk_bar->isVisible())
            chunk_bar->timeElapsed(time);
    }

    QString VideoWidget::formatTime(qint64 cur, qint64 total)
    {
        QTime ct(cur / (60 * 60 * 1000), (cur / (60 * 1000)) % 60, (cur / 1000) % 60, cur % 1000);
        QTime tt(total / (60 * 60 * 1000), (total / (60 * 1000)) % 60, (total / 1000) % 60, total % 1000);
        return QStringLiteral(" %1 / %2 ").arg(ct.toString("hh:mm:ss"), tt.toString("hh:mm:ss"));
    }

    void VideoWidget::playing(const MediaFileRef& mfile)
    {
        bool stream = player->media0bject()->currentSource().type() == Phonon::MediaSource::Stream;
        if (fullscreen && stream)
            chunk_bar->setVisible(slider->isVisible());
        else
            chunk_bar->setVisible(stream);

        chunk_bar->setMediaFile(mfile);
    }

    void VideoWidget::enableActions(unsigned int flags)
    {
        play_action->setEnabled(flags & kt::MEDIA_PLAY);
        stop_action->setEnabled(flags & kt::MEDIA_STOP);
    }

}
