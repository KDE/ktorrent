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
#include <QAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <KToolBar>
#include <KIcon>
#include <KLocale>
#include <KToggleFullScreenAction>
#include <Phonon/Path>
#include <Phonon/AudioOutput>
#include <Phonon/Global>
#include <Phonon/SeekSlider>
#include <Phonon/VolumeSlider>
#include <solid/powermanagement.h>
#include <util/log.h>
#include "videowidget.h"
#include "mediaplayer.h"
#include "screensaver_interface.h"

using namespace bt;

namespace kt
{

	VideoWidget::VideoWidget(MediaPlayer* player,QWidget* parent)
		: QWidget(parent),player(player),fullscreen(false),screensaver_cookie(0),powermanagement_cookie(0)
	{
		QVBoxLayout* vlayout = new QVBoxLayout(this);
		vlayout->setMargin(0);
		vlayout->setSpacing(0);
		video = new Phonon::VideoWidget(this);
		vlayout->addWidget(video);
		
		QHBoxLayout* hlayout = new QHBoxLayout(0);
		
		tb = new KToolBar(this);
		tb->setToolButtonStyle(Qt::ToolButtonIconOnly);
		play_act = tb->addAction(KIcon("media-playback-start"),i18n("Play"),this,SLOT(play()));
		pause_act = tb->addAction(KIcon("media-playback-pause"),i18n("Pause"),this,SLOT(pause()));
		stop_act = tb->addAction(KIcon("media-playback-stop"),i18n("Stop"),this,SLOT(stop()));
		QAction* tfs = tb->addAction(KIcon("view-fullscreen"),i18n("Toggle Fullscreen"));
		tfs->setCheckable(true);
		connect(tfs,SIGNAL(toggled(bool)),this,SIGNAL(toggleFullScreen(bool)));
		hlayout->addWidget(tb);
		
		
		slider = new Phonon::SeekSlider(this);
		slider->setMediaObject(player->media0bject());
		slider->setMaximumHeight(tb->iconSize().height());
		hlayout->addWidget(slider);
		
		
		volume = new Phonon::VolumeSlider(this);
		volume->setAudioOutput(player->output());
		volume->setMaximumHeight(tb->iconSize().height());
		volume->setMaximumWidth(5*tb->iconSize().width());
		hlayout->addWidget(volume);
		
		vlayout->addLayout(hlayout);
	
		Phonon::createPath(player->media0bject(),video);
		
		connect(player->media0bject(),SIGNAL(stateChanged(Phonon::State,Phonon::State)),
				this,SLOT(onStateChanged(Phonon::State, Phonon::State)));
		onStateChanged(player->media0bject()->state(),Phonon::StoppedState);
		
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
	
	void VideoWidget::pause()
	{
		player->media0bject()->pause();
	}
	
	void VideoWidget::stop()
	{
		player->media0bject()->stop();
	}
	
	void VideoWidget::setControlsVisible(bool on)
	{
		if (on)
		{
			slider->show();
			volume->show();
			tb->show();
		}
		else
		{
			slider->hide();
			volume->hide();
			tb->hide();
		}
	}
	
	void VideoWidget::mouseMoveEvent(QMouseEvent* event)
	{
		if (!fullscreen)
			return;
			
		if (slider->isVisible())
		{
			int h = height() - slider->height();
			if (event->y() < h - 10) // use a 10 pixel safety buffer to avoid fibrilation
				setControlsVisible(false);
		}
		else
		{
			int h = height() - slider->height();
			if (event->y() >= h)
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

	void VideoWidget::onStateChanged(Phonon::State cur,Phonon::State old)
	{
		Q_UNUSED(old);
		
		switch (cur)
		{
			case Phonon::LoadingState:
				break;
			case Phonon::ErrorState:
			case Phonon::StoppedState:
				play_act->setEnabled(true);
				pause_act->setEnabled(false);
				stop_act->setEnabled(false);
				break;
			case Phonon::PlayingState:
				play_act->setEnabled(false);
				pause_act->setEnabled(true);
				stop_act->setEnabled(true);
				break;
			case Phonon::BufferingState:
				break; 
			case Phonon::PausedState:
				play_act->setEnabled(true);
				pause_act->setEnabled(false);
				stop_act->setEnabled(true);
				break;
		}
	}
	
	void VideoWidget::inhibitScreenSaver(bool on) 
	{
		QString interface("org.freedesktop.ScreenSaver");
		org::freedesktop::ScreenSaver screensaver(interface, "/ScreenSaver",QDBusConnection::sessionBus());
		if (on)
		{
			QString msg = i18n("KTorrent is playing a video.");
			screensaver_cookie = screensaver.Inhibit("ktorrent",msg);
			Out(SYS_MPL|LOG_NOTICE) << "Screensaver inhibited (cookie " << screensaver_cookie << ")" << endl;
			powermanagement_cookie = Solid::PowerManagement::beginSuppressingSleep(msg);
			Out(SYS_MPL|LOG_NOTICE) << "PowerManagement inhibited (cookie " << powermanagement_cookie << ")" << endl;
		}
		else
		{
			screensaver.UnInhibit(screensaver_cookie);
			powermanagement_cookie = Solid::PowerManagement::stopSuppressingSleep(powermanagement_cookie);
			Out(SYS_MPL|LOG_NOTICE) << "Screensaver uninhibited" << endl;
			Out(SYS_MPL|LOG_NOTICE) << "PowerManagement uninhibited" << endl;
		}
	}

}
