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
#include <interfaces/guiinterface.h>
#include <interfaces/coreinterface.h>
#include "mediaplayerplugin.h"
#include "mediaview.h"
#include "mediamodel.h"
#include "audioplayer.h"

#define NAME "MediaPlayer"
#define AUTHOR "Joris Guisson"
#define EMAIL "joris.guisson@gmail.com"

K_EXPORT_COMPONENT_FACTORY(ktmediaplayerplugin,KGenericFactory<kt::MediaPlayerPlugin>("ktmediaplayerplugin"))
		
using namespace bt;

namespace kt
{

	MediaPlayerPlugin::MediaPlayerPlugin(QObject* parent, const QStringList& args)
	: Plugin(parent,NAME,i18n("MediaPlayer"),AUTHOR,EMAIL, i18n("Phonon based media player plugin for KTorrent"),"applications-multimedia")
	{
		media_view = 0;
		media_model = 0;
		audio_player = 0;
		action_flags = 0;
		play_action = pause_action = stop_action = prev_action = next_action = 0;
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
		audio_player = new AudioPlayer(this);
		media_view = new MediaView(audio_player,media_model,0);
	
		getGUI()->addToolWidget(media_view,"applications-multimedia",i18n("Media Player"),GUIInterface::DOCK_LEFT);
		
		connect(core,SIGNAL(torrentAdded(bt::TorrentInterface*)),media_model,SLOT(onTorrentAdded(bt::TorrentInterface*)));
		connect(core,SIGNAL(torrentRemoved(bt::TorrentInterface*)),media_model,SLOT(onTorrentRemoved(bt::TorrentInterface*)));
		connect(audio_player,SIGNAL(enableActions(unsigned int)),this,SLOT(enableActions(unsigned int)));
		connect(media_view,SIGNAL(selectionChanged(const QModelIndex &)),this,SLOT(onSelectionChanged(const QModelIndex&)));
	
		setupActions();
		setXMLFile("ktmediaplayerpluginui.rc");
		enableActions(kt::MEDIA_PLAY);
	}
	
	void MediaPlayerPlugin::unload()
	{
		getGUI()->removeToolWidget(media_view);
		delete media_view;
		media_view = 0;
		delete media_model;
		media_model = 0;
		delete audio_player;
		audio_player = 0;
	}
	
	bool MediaPlayerPlugin::versionCheck(const QString& version) const
	{
		return version == KT_VERSION_MACRO;
	}
	
	void MediaPlayerPlugin::play()
	{
		Out(SYS_GEN|LOG_DEBUG) << "MediaPlayerPlugin::play " << endl;
		QModelIndex idx = media_view->selectedItem();
		if (idx.isValid())
		{
			Out(SYS_GEN|LOG_DEBUG) << "MediaPlayerPlugin::play " << idx.row() << endl;
			QString path = media_model->pathForIndex(idx);
			if (bt::Exists(path))
			{
				Out(SYS_GEN|LOG_DEBUG) << "MediaPlayerPlugin::play " << path << endl;
				audio_player->play(path);
			}
		}
	}
	
	void MediaPlayerPlugin::pause()
	{
		audio_player->pause();
	}
	
	void MediaPlayerPlugin::stop()
	{
		audio_player->stop();
	}
	
	void MediaPlayerPlugin::prev()
	{
		audio_player->prev();
	}
	
	void MediaPlayerPlugin::next()
	{
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
				play_action->setEnabled((flags & kt::MEDIA_PLAY) || path != audio_player->getCurrentSource());
		}
		prev_action->setEnabled(flags & kt::MEDIA_PREV);
		
		action_flags = flags;
	}
	
	void MediaPlayerPlugin::onSelectionChanged(const QModelIndex & idx)
	{
		if (idx.isValid())
		{
			QString path = media_model->pathForIndex(idx);
			if (bt::Exists(path))
				play_action->setEnabled((action_flags & kt::MEDIA_PLAY) || path != audio_player->getCurrentSource());
			
		}
	}
}
