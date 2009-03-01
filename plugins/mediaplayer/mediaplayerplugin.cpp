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

#include <kgenericfactory.h>
#include <kicon.h>
#include <klocale.h>
#include <kaction.h>
#include <ktoggleaction.h>
#include <kactioncollection.h>

#include <util/log.h>
#include <util/logsystemmanager.h>
#include <util/fileops.h>
#include <util/functions.h>
#include <interfaces/guiinterface.h>
#include <interfaces/coreinterface.h>
#include "mediaplayerplugin.h"
#include "mediaview.h"
#include "mediamodel.h"
#include "mediaplayer.h"
#include "videowidget.h"
#include "mediaplayerpluginsettings.h"


K_EXPORT_COMPONENT_FACTORY(ktmediaplayerplugin,KGenericFactory<kt::MediaPlayerPlugin>("ktmediaplayerplugin"))
		
using namespace bt;

namespace kt
{

	MediaPlayerPlugin::MediaPlayerPlugin(QObject* parent, const QStringList& args) : Plugin(parent)
	{
		Q_UNUSED(args);
		media_view = 0;
		media_model = 0;
		media_player = 0;
		action_flags = 0;
		play_action = pause_action = stop_action = prev_action = next_action = 0;
		video = 0;
		fullscreen_mode = false;
		fs_dialog = 0;
	}


	MediaPlayerPlugin::~MediaPlayerPlugin()
	{
	}
	
	void MediaPlayerPlugin::setupActions()
	{
		KActionCollection* ac = actionCollection();
		
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
		
		QToolBar* tb = media_view->mediaToolBar();
		tb->addAction(play_action);
		tb->addAction(pause_action);
		tb->addAction(stop_action);
		tb->addAction(prev_action);
		tb->addAction(next_action);
		tb->addAction(show_video_action);
	}

	void MediaPlayerPlugin::load()
	{
		LogSystemManager::instance().registerSystem(i18n("Media Player"),SYS_MPL);
		CoreInterface* core = getCore();
		media_model = new MediaModel(getCore(),this);
		media_player = new MediaPlayer(this);
		media_view = new MediaView(media_player,media_model,0);
	
		getGUI()->addToolWidget(media_view,"applications-multimedia",i18n("Media Player"),i18n("The media player allows you to play music and movies inside KTorrent"),GUIInterface::DOCK_LEFT);
		
		connect(core,SIGNAL(torrentAdded(bt::TorrentInterface*)),media_model,SLOT(onTorrentAdded(bt::TorrentInterface*)));
		connect(core,SIGNAL(torrentRemoved(bt::TorrentInterface*)),media_model,SLOT(onTorrentRemoved(bt::TorrentInterface*)));
		connect(media_player,SIGNAL(enableActions(unsigned int)),this,SLOT(enableActions(unsigned int)));
		connect(media_player,SIGNAL(openVideo()),this,SLOT(openVideo()));
		connect(media_player,SIGNAL(closeVideo()),this,SLOT(closeVideo()));
		connect(media_player,SIGNAL(aboutToFinish()),this,SLOT(aboutToFinishPlaying()));
		connect(media_player,SIGNAL(playing(QString)),media_view,SLOT(playing(QString)));
		connect(media_view,SIGNAL(selectionChanged(const QModelIndex &)),this,SLOT(onSelectionChanged(const QModelIndex&)));
		connect(media_view,SIGNAL(doubleClicked(const QModelIndex&)),this,SLOT(onDoubleClicked(const QModelIndex&)));
		connect(media_view,SIGNAL(randomModeActivated()),this,SLOT(randomPlayActivated()));
	
		setupActions();
		setXMLFile("ktmediaplayerpluginui.rc");
		enableActions(0);
	}
	
	void MediaPlayerPlugin::unload()
	{
		LogSystemManager::instance().unregisterSystem(i18n("Media Player"));
		if (fullscreen_mode)
			setVideoFullScreen(false);
		
		closeVideo();
		
		getGUI()->removeToolWidget(media_view);
		delete media_view;
		media_view = 0;
		delete media_model;
		media_model = 0;
		delete media_player;
		media_player = 0;
	}
	
	bool MediaPlayerPlugin::versionCheck(const QString& version) const
	{
		return version == KT_VERSION_MACRO;
	}
	
	void MediaPlayerPlugin::openVideo()
	{
		QString path = media_player->media0bject()->currentSource().fileName();
		int idx = path.lastIndexOf(bt::DirSeparator());
		if (idx >= 0)
			path = path.mid(idx+1);
		
		if (path.isNull())
			path = i18n("Media Player");
		
		if (video)
		{
			getGUI()->setTabText(video,path);
		}
		else
		{
			video = new VideoWidget(media_player,0);
			connect(video,SIGNAL(toggleFullScreen(bool)),this,SLOT(setVideoFullScreen(bool)));
			getGUI()->addTabPage(video,"video-x-generic",path,i18n("Movie player"),this);
		}
		
		if (!show_video_action->isChecked())
			show_video_action->setChecked(true);
	}
	
	void MediaPlayerPlugin::closeVideo()
	{
		if (video)
		{
			getGUI()->removeTabPage(video);
			show_video_action->setChecked(false);
			video->deleteLater();
			video = 0;
		}
	}
	
	void MediaPlayerPlugin::showVideo(bool on)
	{
		if (on)
			openVideo();
		else
			closeVideo();
	}
	
	void MediaPlayerPlugin::play()
	{
		if (media_player->paused())
			media_player->resume();
		else
			onDoubleClicked(media_view->selectedItem());
	}
	
	void MediaPlayerPlugin::onDoubleClicked(const QModelIndex & idx)
	{
		if (idx.isValid())
		{
			QString path = media_model->pathForIndex(idx);
			if (bt::Exists(path))
			{
				Out(SYS_GEN|LOG_DEBUG) << "MediaPlayer: playing " << path << endl;
				media_player->play(path);
				curr_item = idx;
				bool random = MediaPlayerPluginSettings::playMode() == 2;
				QModelIndex next = media_model->next(curr_item,random,MediaPlayerPluginSettings::skipIncomplete());
				next_action->setEnabled(next.isValid());
			}
		}
	}
	
	void MediaPlayerPlugin::pause()
	{
		media_player->pause();
	}
	
	void MediaPlayerPlugin::stop()
	{
		media_player->stop();
	}
	
	void MediaPlayerPlugin::prev()
	{
		QString s = media_player->prev();
		if (s.isNull())
			return;
		
		curr_item = media_model->indexForPath(s);
	}
	
	void MediaPlayerPlugin::next()
	{
		bool random = MediaPlayerPluginSettings::playMode() == 2;
		QModelIndex n = media_model->next(curr_item,random,MediaPlayerPluginSettings::skipIncomplete());
		if (!n.isValid())
			return;
		
		QString path = media_model->pathForIndex(n);
		if (bt::Exists(path))
		{
			media_player->play(path);
			curr_item = n;
			n = media_model->next(curr_item,random,MediaPlayerPluginSettings::skipIncomplete());
			next_action->setEnabled(n.isValid());
		}
	}
	
	void MediaPlayerPlugin::enableActions(unsigned int flags)
	{
		pause_action->setEnabled(flags & kt::MEDIA_PAUSE);
		stop_action->setEnabled(flags & kt::MEDIA_STOP);
		
		play_action->setEnabled(false);
		
		QModelIndex idx = media_view->selectedItem();
		if (idx.isValid())
		{
			QString path = media_model->pathForIndex(idx);
			if (bt::Exists(path))
				play_action->setEnabled((flags & kt::MEDIA_PLAY) || path != media_player->getCurrentSource());
			else
				play_action->setEnabled(action_flags & kt::MEDIA_PLAY);
		}
		else
			play_action->setEnabled(flags & kt::MEDIA_PLAY);
		
		prev_action->setEnabled(flags & kt::MEDIA_PREV);
		
		action_flags = flags;
	}
	
	void MediaPlayerPlugin::onSelectionChanged(const QModelIndex & idx)
	{
		if (idx.isValid())
		{
			QString path = media_model->pathForIndex(idx);
			if (bt::Exists(path))
				play_action->setEnabled((action_flags & kt::MEDIA_PLAY) || path != media_player->getCurrentSource());
			else
				play_action->setEnabled(action_flags & kt::MEDIA_PLAY);
		}
		else
			play_action->setEnabled(action_flags & kt::MEDIA_PLAY);
	}
	
	void MediaPlayerPlugin::randomPlayActivated()
	{
		QModelIndex next = media_model->next(curr_item,true,MediaPlayerPluginSettings::skipIncomplete());
		next_action->setEnabled(next.isValid());
	}
	
	void MediaPlayerPlugin::aboutToFinishPlaying()
	{
		if (MediaPlayerPluginSettings::playMode() == 0)
			return;
		
		bool random = MediaPlayerPluginSettings::playMode() == 2;
		QModelIndex n = media_model->next(curr_item,random,MediaPlayerPluginSettings::skipIncomplete());
		if (!n.isValid())
			return;
		
		QString path = media_model->pathForIndex(n);
		if (bt::Exists(path))
		{
			media_player->queue(path);
			curr_item = n;
			n = media_model->next(curr_item,random,MediaPlayerPluginSettings::skipIncomplete());
			next_action->setEnabled(n.isValid());
		}
	}
	
	void MediaPlayerPlugin::tabCloseRequest(kt::GUIInterface* gui,QWidget* tab)
	{
		if (video != tab)
			return;
		
		stop();
		closeVideo();
	}
	
	void MediaPlayerPlugin::setVideoFullScreen(bool on)
	{
		if (!video)
			return;
		
		if (on && !fullscreen_mode)
		{
			getGUI()->removeTabPage(video);
			video->setParent(0);
			video->setFullScreen(true);
			video->show();
			fullscreen_mode = true;
		}
		else if (!on && fullscreen_mode)
		{
			video->hide();
			video->setFullScreen(false);
			
			QString path = media_player->media0bject()->currentSource().fileName();
			int idx = path.lastIndexOf(bt::DirSeparator());
			if (idx >= 0)
				path = path.mid(idx+1);
			
			getGUI()->addTabPage(video,"video-x-generic",path,i18n("Movie player"),this);
			fullscreen_mode = false;
		}
	}
}
