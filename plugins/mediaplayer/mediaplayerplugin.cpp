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
#include <kactioncollection.h>

#include <util/log.h>
#include <util/fileops.h>
#include <util/functions.h>
#include <interfaces/guiinterface.h>
#include <interfaces/coreinterface.h>
#include "mediaplayerplugin.h"
#include "mediaview.h"
#include "mediamodel.h"
#include "mediaplayer.h"
#include "videowidget.h"


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
		video_shown = false;
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
		
		QToolBar* tb = media_view->mediaToolBar();
		tb->addAction(play_action);
		tb->addAction(pause_action);
		tb->addAction(stop_action);
		tb->addAction(prev_action);
		tb->addAction(next_action);
	}

	void MediaPlayerPlugin::load()
	{
		CoreInterface* core = getCore();
		media_model = new MediaModel(getCore(),this);
		media_player = new MediaPlayer(this);
		media_view = new MediaView(media_player,media_model,0);
	
		getGUI()->addToolWidget(media_view,"applications-multimedia",i18n("Media Player"),GUIInterface::DOCK_LEFT);
		
		connect(core,SIGNAL(torrentAdded(bt::TorrentInterface*)),media_model,SLOT(onTorrentAdded(bt::TorrentInterface*)));
		connect(core,SIGNAL(torrentRemoved(bt::TorrentInterface*)),media_model,SLOT(onTorrentRemoved(bt::TorrentInterface*)));
		connect(media_player,SIGNAL(enableActions(unsigned int)),this,SLOT(enableActions(unsigned int)));
		connect(media_player,SIGNAL(openVideo()),this,SLOT(openVideo()));
		connect(media_player,SIGNAL(closeVideo()),this,SLOT(closeVideo()));
		connect(media_view,SIGNAL(selectionChanged(const QModelIndex &)),this,SLOT(onSelectionChanged(const QModelIndex&)));
		connect(media_view,SIGNAL(doubleClicked(const QModelIndex&)),this,SLOT(onDoubleClicked(const QModelIndex&)));
	
		setupActions();
		setXMLFile("ktmediaplayerpluginui.rc");
		enableActions(0);
	}
	
	void MediaPlayerPlugin::unload()
	{
		if (fullscreen_mode)
			setVideoFullScreen(false);
		
		closeVideo();
		delete video;
		video = 0;
		
		getGUI()->removeToolWidget(media_view);
		delete media_view;
		media_view = 0;
		delete media_model;
		media_model = 0;
		delete media_player;
		media_player = 0;
		video_shown = false;
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
		
		if (video)
		{
			if (video_shown)
				getGUI()->setTabText(video,path);
			else
				getGUI()->addTabPage(video,"video-x-generic",path,this);
		}
		else
		{
			video = new VideoWidget(media_player,0);
			connect(video,SIGNAL(toggleFullScreen(bool)),this,SLOT(setVideoFullScreen(bool)));
			getGUI()->addTabPage(video,"video-x-generic",path,this);
		}
		video_shown = true;
	}
	
	void MediaPlayerPlugin::closeVideo()
	{
		if (video)
		{
			getGUI()->removeTabPage(video);
			video_shown = false;
		}
	}
	
	void MediaPlayerPlugin::play()
	{
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
				next_action->setEnabled(media_model->next(curr_item).isValid());
				media_view->playing(curr_item);
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
		media_view->playing(curr_item);
	}
	
	void MediaPlayerPlugin::next()
	{
		QModelIndex n = media_model->next(curr_item);
		if (!n.isValid())
			return;
		
		QString path = media_model->pathForIndex(n);
		if (bt::Exists(path))
		{
			Out(SYS_GEN|LOG_DEBUG) << "MediaPlayer: playing " << path << endl;
			media_player->play(path);
			curr_item = n;
			next_action->setEnabled(media_model->next(curr_item).isValid());
			media_view->playing(curr_item);
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
	
	void MediaPlayerPlugin::tabCloseRequest(kt::GUIInterface* gui,QWidget* tab)
	{
		if (video != tab)
			return;
		
		stop();
		gui->removeTabPage(video);
		video_shown = false;
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
			
			getGUI()->addTabPage(video,"video-x-generic",path,this);
			fullscreen_mode = false;
		}
	}
}
