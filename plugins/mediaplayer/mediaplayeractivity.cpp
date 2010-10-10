/***************************************************************************
*   Copyright (C) 2009 by Joris Guisson                                   *
*   joris.guisson@gmail.com                                               *
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
#include <QBoxLayout>
#include <kicon.h>
#include <klocale.h>
#include <kaction.h>
#include <ktoggleaction.h>
#include <kactioncollection.h>

#include <util/log.h>
#include <util/fileops.h>
#include <util/functions.h>
#include <interfaces/coreinterface.h>
#include "mediaview.h"
#include "mediamodel.h"
#include "mediaplayer.h"
#include "videowidget.h"
#include "mediaplayerpluginsettings.h"
#include "mediaplayeractivity.h"
#include <QToolButton>
#include "playlistwidget.h"
#include "playlist.h"
#include <interfaces/functions.h>

using namespace bt;

namespace kt
{
	MediaPlayerActivity::MediaPlayerActivity(CoreInterface* core,QWidget* parent) 
		: Activity(i18n("Media Player"),"applications-multimedia",90,parent)
	{
		action_flags = 0;
		video = 0;
		play_action = pause_action = stop_action = prev_action = next_action = 0;
		fullscreen_mode = false;
		
		media_model = new MediaModel(core,this);
		media_player = new MediaPlayer(this);
		
		QHBoxLayout* layout = new QHBoxLayout(this);
		layout->setMargin(0);
		
		splitter = new QSplitter(Qt::Horizontal,this);
		layout->addWidget(splitter);
		media_view = new MediaView(media_model,splitter);
		tabs = new KTabWidget(splitter);
		splitter->addWidget(media_view);
		splitter->addWidget(tabs);
		
		close_button = new QToolButton(tabs);
		tabs->setCornerWidget(close_button,Qt::TopRightCorner);
		close_button->setIcon(KIcon("tab-close"));
		close_button->setEnabled(false);
		connect(close_button,SIGNAL(clicked()),this,SLOT(closeTab()));
		
		play_list = new PlayListWidget(media_model,media_player,tabs);
		tabs->addTab(play_list,KIcon("audio-x-generic"),i18n("Play List"));
		tabs->setTabBarHidden(true);
		
		connect(core,SIGNAL(torrentAdded(bt::TorrentInterface*)),media_model,SLOT(onTorrentAdded(bt::TorrentInterface*)));
		connect(core,SIGNAL(torrentRemoved(bt::TorrentInterface*)),media_model,SLOT(onTorrentRemoved(bt::TorrentInterface*)));
		connect(media_player,SIGNAL(enableActions(unsigned int)),this,SLOT(enableActions(unsigned int)));
		connect(media_player,SIGNAL(openVideo()),this,SLOT(openVideo()));
		connect(media_player,SIGNAL(closeVideo()),this,SLOT(closeVideo()));
		connect(media_player,SIGNAL(aboutToFinish()),this,SLOT(aboutToFinishPlaying()));
		connect(play_list,SIGNAL(selectionChanged(const QModelIndex &)),this,SLOT(onSelectionChanged(const QModelIndex&)));
		connect(media_view,SIGNAL(doubleClicked(const MediaFileRef&)),this,SLOT(onDoubleClicked(const MediaFileRef&)));
		connect(play_list,SIGNAL(randomModeActivated()),this,SLOT(randomPlayActivated()));
		connect(play_list,SIGNAL(doubleClicked(MediaFileRef)),this,SLOT(play(MediaFileRef)));
		connect(tabs,SIGNAL(currentChanged(int)),this,SLOT(currentTabChanged(int)));
	}

	MediaPlayerActivity::~MediaPlayerActivity() 
	{
		if (fullscreen_mode)
			setVideoFullScreen(false);
	}
	
	void MediaPlayerActivity::setupActions(KActionCollection* ac)
	{
		play_action = new KAction(KIcon("media-playback-start"),i18n("Play"),this);
		connect(play_action,SIGNAL(triggered()),this,SLOT(play()));
		ac->addAction("media_play",play_action);
		
		pause_action = new KAction(KIcon("media-playback-pause"),i18n("Pause"),this);
		connect(pause_action,SIGNAL(triggered()),this,SLOT(pause()));
		ac->addAction("media_pause",pause_action);
		
		stop_action = new KAction(KIcon("media-playback-stop"),i18n("Stop"),this);
		connect(stop_action,SIGNAL(triggered()),this,SLOT(stop()));
		ac->addAction("media_stop",stop_action);
		
		prev_action = new KAction(KIcon("media-skip-backward"),i18n("Previous"),this);
		connect(prev_action,SIGNAL(triggered()),this,SLOT(prev()));
		ac->addAction("media_prev",prev_action);
		
		next_action = new KAction(KIcon("media-skip-forward"),i18n("Next"),this);
		connect(next_action,SIGNAL(triggered()),this,SLOT(next()));
		ac->addAction("media_next",next_action);
		
		show_video_action = new KToggleAction(KIcon("video-x-generic"),i18n("Show Video"),this);
		connect(show_video_action,SIGNAL(toggled(bool)),this,SLOT(showVideo(bool)));
		ac->addAction("show_video",show_video_action);
		
		add_media_action = new KAction(KIcon("document-open"),i18n("Add Media"),this);
		connect(add_media_action,SIGNAL(triggered()),play_list,SLOT(addMedia()));
		ac->addAction("add_media",add_media_action); 
		
		clear_action = new KAction(KIcon("edit-clear-list"),i18n("Clear Playlist"),this);
		connect(clear_action,SIGNAL(triggered()),play_list,SLOT(clearPlayList()));
		ac->addAction("clear_play_list",clear_action);
		
		QToolBar* tb = play_list->mediaToolBar();
		tb->addAction(add_media_action);
		tb->addAction(clear_action);
		tb->addSeparator();
		tb->addAction(play_action);
		tb->addAction(pause_action);
		tb->addAction(stop_action);
		tb->addAction(prev_action);
		tb->addAction(next_action);
		tb->addSeparator();
		tb->addAction(show_video_action);
	}

	void MediaPlayerActivity::openVideo()
	{
		QString path = media_player->getCurrentSource().path();
		int idx = path.lastIndexOf(bt::DirSeparator());
		if (idx >= 0)
			path = path.mid(idx+1);
		
		if (path.isEmpty())
			path = i18n("Media Player");
		
		if (video)
		{
			int idx = tabs->indexOf(video);
			tabs->setTabText(idx,path);
			tabs->setCurrentIndex(idx);
			tabs->setTabBarHidden(false);
		}
		else
		{
			video = new VideoWidget(media_player,0);
			connect(video,SIGNAL(toggleFullScreen(bool)),this,SLOT(setVideoFullScreen(bool)));
			int idx = tabs->addTab(video,KIcon("video-x-generic"),path);
			tabs->setTabToolTip(idx,i18n("Movie player"));
			tabs->setCurrentIndex(idx);
			tabs->setTabBarHidden(false);
		}
		
		if (!show_video_action->isChecked())
			show_video_action->setChecked(true);
	}

	void MediaPlayerActivity::closeVideo()
	{
		if (video)
		{
			tabs->removePage(video);
			if (show_video_action->isChecked())
				show_video_action->setChecked(false);
			tabs->setTabBarHidden(true);
			video->deleteLater();
			video = 0;
		}
	}

	void MediaPlayerActivity::showVideo(bool on)
	{
		if (on)
			openVideo();
		else
			closeVideo();
	}

	void MediaPlayerActivity::play()
	{
		if (media_player->paused())
		{
			media_player->resume();
		}
		else
		{
			curr_item = play_list->play();
			if (curr_item.isValid())
			{
				bool random = MediaPlayerPluginSettings::playMode() == 2;
				QModelIndex n = play_list->next(curr_item,random);
				next_action->setEnabled(n.isValid());
			}
		}
	}
	
	void MediaPlayerActivity::play(const MediaFileRef & file)
	{
		media_player->play(file);
		QModelIndex idx = play_list->indexForFile(file.path());
		if (idx.isValid())
		{
			curr_item = idx;
			bool random = MediaPlayerPluginSettings::playMode() == 2;
			QModelIndex n = play_list->next(curr_item,random);
			next_action->setEnabled(n.isValid());
		}
	}

	void MediaPlayerActivity::onDoubleClicked(const MediaFileRef & file)
	{
		if (bt::Exists(file.path()))
		{
			play(file);
		}
	}

	void MediaPlayerActivity::pause()
	{
		media_player->pause();
	}

	void MediaPlayerActivity::stop()
	{
		media_player->stop();
	}

	void MediaPlayerActivity::prev()
	{
		media_player->prev();
	}

	void MediaPlayerActivity::next()
	{
		bool random = MediaPlayerPluginSettings::playMode() == 2;
		QModelIndex n = play_list->next(curr_item,random);
		if (!n.isValid())
			return;
		
		QString path = play_list->fileForIndex(n);
		if (bt::Exists(path))
		{
			media_player->play(path);
			curr_item = n;
			n = play_list->next(curr_item,random);
			next_action->setEnabled(n.isValid());
		}
	}

	void MediaPlayerActivity::enableActions(unsigned int flags)
	{
		pause_action->setEnabled(flags & kt::MEDIA_PAUSE);
		stop_action->setEnabled(flags & kt::MEDIA_STOP);
		play_action->setEnabled(false);
		
		QModelIndex idx = play_list->selectedItem();
		if (idx.isValid())
		{
			PlayList* pl = play_list->playList();
			MediaFileRef file = pl->fileForIndex(idx);
			if (bt::Exists(file.path()))
				play_action->setEnabled((flags & kt::MEDIA_PLAY) || file != media_player->getCurrentSource());
			else
				play_action->setEnabled(action_flags & kt::MEDIA_PLAY);
		}
		else
			play_action->setEnabled(flags & kt::MEDIA_PLAY);
		
		prev_action->setEnabled(flags & kt::MEDIA_PREV);
		action_flags = flags;
	}

	void MediaPlayerActivity::onSelectionChanged(const QModelIndex & idx)
	{
		if (idx.isValid())
		{
			PlayList* pl = play_list->playList();
			MediaFileRef file = pl->fileForIndex(idx);
			if (bt::Exists(file.path()))
				play_action->setEnabled((action_flags & kt::MEDIA_PLAY) || file != media_player->getCurrentSource());
			else
				play_action->setEnabled(action_flags & kt::MEDIA_PLAY);
		}
		else
			play_action->setEnabled(action_flags & kt::MEDIA_PLAY);
	}

	void MediaPlayerActivity::randomPlayActivated()
	{
		QModelIndex next = play_list->next(curr_item,true);
		next_action->setEnabled(next.isValid());
	}

	void MediaPlayerActivity::aboutToFinishPlaying()
	{
		if (MediaPlayerPluginSettings::playMode() == 0)
			return;
		
		bool random = MediaPlayerPluginSettings::playMode() == 2;
		QModelIndex n = play_list->next(curr_item,random);
		if (!n.isValid())
			return;
		
		QString path = play_list->fileForIndex(n);
		if (bt::Exists(path))
		{
			media_player->queue(path);
			curr_item = n;
			n = play_list->next(curr_item,random);
			next_action->setEnabled(n.isValid());
		}
	}

	void MediaPlayerActivity::closeTab()
	{
		if (video != tabs->currentWidget())
			return;
		
		stop();
		closeVideo();
	}

	void MediaPlayerActivity::setVideoFullScreen(bool on)
	{
		if (!video)
			return;
		
		if (on && !fullscreen_mode)
		{
			tabs->removePage(video);
			video->setParent(0);
			video->setFullScreen(true);
			video->show();
			fullscreen_mode = true;
		}
		else if (!on && fullscreen_mode)
		{
			video->hide();
			video->setFullScreen(false);
			
			QString path = media_player->getCurrentSource().path();
			int idx = path.lastIndexOf(bt::DirSeparator());
			if (idx >= 0)
				path = path.mid(idx+1);
			
			if (path.isEmpty())
				path = i18n("Media Player");
			
			idx = tabs->addTab(video,KIcon("video-x-generic"),path);
			tabs->setTabToolTip(idx,i18n("Movie player"));
			tabs->setCurrentIndex(idx);
			fullscreen_mode = false;
		}
	}

	void MediaPlayerActivity::saveState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("MediaPlayerActivity");
		g.writeEntry("splitter_state",splitter->saveState());
		play_list->saveState(cfg);
		play_list->playList()->save(kt::DataDir() + "playlist");
	}
	
	void MediaPlayerActivity::loadState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("MediaPlayerActivity");
		QByteArray d = g.readEntry("splitter_state",QByteArray());
		if (!d.isNull())
			splitter->restoreState(d);
		
		play_list->loadState(cfg);
		if (bt::Exists(kt::DataDir() + "playlist"))
			play_list->playList()->load(kt::DataDir() + "playlist");
	}

	void MediaPlayerActivity::currentTabChanged(int idx)
	{
		close_button->setEnabled(idx != 0);
	}
	
}

