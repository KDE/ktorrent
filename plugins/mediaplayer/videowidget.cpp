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
#include <KToolBar>
#include <KIcon>
#include <KLocale>
#include <Phonon/Path>
#include <Phonon/AudioOutput>
#include <Phonon/Global>
#include <Phonon/SeekSlider>
#include <Phonon/VolumeSlider>
#include "videowidget.h"
#include "mediaplayer.h"

namespace kt
{

	VideoWidget::VideoWidget(MediaPlayer* player,QWidget* parent)
			: QWidget(parent),player(player)
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
		connect(tfs,SIGNAL(toggled(bool)),this,SLOT(toggleFullScreen(bool)));
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
	}


	VideoWidget::~VideoWidget()
	{
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
	
	void VideoWidget::toggleFullScreen(bool on)
	{
		Q_UNUSED(on);
		setWindowState(windowState() ^ Qt::WindowFullScreen);
	}

}
