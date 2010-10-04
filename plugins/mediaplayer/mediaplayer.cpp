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

#include <Phonon/Path>
#include <Phonon/AudioOutput>
#include <Phonon/Global>

#include <util/log.h>
#include <torrent/torrentfilestream.h>
#include "mediaplayer.h"

using namespace bt;

namespace kt
{

	MediaPlayer::MediaPlayer(QObject* parent)
			: QObject(parent)
	{
		media = new Phonon::MediaObject(this);
		audio = new Phonon::AudioOutput(Phonon::MusicCategory, this);
		Phonon::createPath(media,audio);
		
		connect(media,SIGNAL(stateChanged(Phonon::State,Phonon::State)),
				this,SLOT(onStateChanged(Phonon::State, Phonon::State)));
		connect(media,SIGNAL(hasVideoChanged(bool)),this,SLOT(hasVideoChanged(bool)));
		connect(media,SIGNAL(aboutToFinish()),this,SIGNAL(aboutToFinish()));
		connect(media,SIGNAL(currentSourceChanged(Phonon::MediaSource)),
				 this,SLOT(currentSourceChanged(Phonon::MediaSource)));
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
		if (paused())
			media->play();
	}	

	void MediaPlayer::play(kt::MediaFileRef file)
	{
		if (media->state() == Phonon::PausedState)
		{
			media->play();
		}
		else
		{
			Out(SYS_MPL|LOG_NOTICE) << "MediaPlayer: playing " << file.path() << endl;
			media->setCurrentSource(file.createMediaSource());
			media->play();
			history.append(file);
		}
	}

	
	void MediaPlayer::queue(kt::MediaFileRef file)
	{
		Out(SYS_MPL|LOG_NOTICE) << "MediaPlayer: enqueue " << file.path() << endl;
		media->enqueue(file.createMediaSource());
		history.append(file);
		onStateChanged(media->state(),Phonon::StoppedState);
	}
		
	void MediaPlayer::pause()
	{
		media->pause();
	}
		
	void MediaPlayer::stop()
	{
		media->stop();
		media->clear();
	}
	
	MediaFileRef MediaPlayer::prev()
	{
		if (media->state() == Phonon::PausedState || media->state() == Phonon::PlayingState)
		{
			if (history.count() >= 2)
			{
				history.pop_back(); // remove the currently playing file
				MediaFileRef & file = history.back();
				media->setCurrentSource(file.createMediaSource());
				media->play();
				Out(SYS_MPL|LOG_NOTICE) << "MediaPlayer: playing previous file " << file.path() << endl;
				return file;
			}
		}
		else 
		{
			if (history.count() > 0)
			{
				MediaFileRef & file = history.back();
				media->setCurrentSource(file.createMediaSource());
				media->play();
				Out(SYS_MPL|LOG_NOTICE) << "MediaPlayer: playing previous file " << file.path() << endl;
				return file;
			}
		}
		
		return QString();
	}
	
	void MediaPlayer::onStateChanged(Phonon::State cur, Phonon::State)
	{
		unsigned int flags = 0;
		switch (cur)
		{
			case Phonon::LoadingState:
				Out(SYS_MPL|LOG_DEBUG) << "MediaPlayer: loading" << endl;
				break;
			case Phonon::StoppedState:
				Out(SYS_MPL|LOG_DEBUG) << "MediaPlayer: stopped" << endl;
				flags = MEDIA_PLAY;
				if (history.count() > 0)
					flags |= MEDIA_PREV;
			
				enableActions(flags);
				stopped();
				break;
			case Phonon::PlayingState:
				Out(SYS_MPL|LOG_DEBUG) << "MediaPlayer: playing " << getCurrentSource().path() << endl;
				flags = MEDIA_PAUSE|MEDIA_STOP;
				if (history.count() > 1)
					flags |= MEDIA_PREV;
				
				enableActions(flags);
				hasVideoChanged(media->hasVideo());
				playing(getCurrentSource());
				break;
			case Phonon::BufferingState:
				Out(SYS_MPL|LOG_DEBUG) << "MediaPlayer: buffering" << endl;
				break; 
			case Phonon::PausedState:
				Out(SYS_MPL|LOG_DEBUG) << "MediaPlayer: paused" << endl;
				flags = MEDIA_PLAY|MEDIA_STOP;
				if (history.count() > 1)
					flags |= MEDIA_PREV;
				
				enableActions(flags);
				break;
			case Phonon::ErrorState:
				Out(SYS_MPL|LOG_IMPORTANT) << "MediaPlayer: error " << media->errorString() << endl;
				flags = MEDIA_PLAY;
				if (history.count() > 0)
					flags |= MEDIA_PREV;
		
				enableActions(flags);
				break;
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
	
	
	void MediaPlayer::currentSourceChanged(Phonon::MediaSource src)
	{
		playing(MediaFileRef(src.fileName()));
	}
}
