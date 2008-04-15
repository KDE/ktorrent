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
		media->setTickInterval(1000);
	}


	MediaPlayer::~MediaPlayer()
	{
		media->stop();
	}

	void MediaPlayer::play(const QString & file)
	{
		if (media->state() == Phonon::PausedState)
		{
			media->play();
		}
		else
		{
			media->setCurrentSource(file);
			media->play();
			history.append(file);
		}
	}
		
	void MediaPlayer::pause()
	{
		media->pause();
	}
		
	void MediaPlayer::stop()
	{
		media->stop();
	}
	
	void MediaPlayer::prev()
	{
		if (media->state() == Phonon::PausedState || media->state() == Phonon::PlayingState)
		{
			if (history.count() >= 2)
			{
				history.pop_back(); // remove the currently playing file
				QString file = history.back();
				media->setCurrentSource(file);
				media->play();
			}
		}
		else if (history.count() > 0)
		{
			QString file = history.back();
			media->setCurrentSource(file);
			media->play();
		}
	}
	
	void MediaPlayer::onStateChanged(Phonon::State cur, Phonon::State)
	{
		unsigned int flags = 0;
		switch (cur)
		{
			case Phonon::LoadingState:
				Out(SYS_GEN|LOG_DEBUG) << "MediaPlayer: loading" << endl;
				break;
			case Phonon::StoppedState:
				Out(SYS_GEN|LOG_DEBUG) << "MediaPlayer: stopped" << endl;
				flags = MEDIA_PLAY;
				if (history.count() > 0)
					flags |= MEDIA_PREV;
			
				enableActions(flags);
				break;
			case Phonon::PlayingState:
				Out(SYS_GEN|LOG_DEBUG) << "MediaPlayer: playing" << endl;
				flags = MEDIA_PAUSE|MEDIA_STOP;
				if (history.count() > 1)
					flags |= MEDIA_PREV;
				
				enableActions(flags);
				break;
			case Phonon::BufferingState:
				Out(SYS_GEN|LOG_DEBUG) << "MediaPlayer: buffering" << endl;
				break; 
			case Phonon::PausedState:
				Out(SYS_GEN|LOG_DEBUG) << "MediaPlayer: paused" << endl;
				flags = MEDIA_PLAY|MEDIA_STOP;
				if (history.count() > 1)
					flags |= MEDIA_PREV;
				
				enableActions(flags);
				break;
			case Phonon::ErrorState:
				Out(SYS_GEN|LOG_DEBUG) << "MediaPlayer: error " << media->errorString() << endl;
				flags = MEDIA_PLAY;
				if (history.count() > 0)
					flags |= MEDIA_PREV;
		
				enableActions(flags);
				break;
		}
		
		
	}
	
	QString MediaPlayer::getCurrentSource() const
	{
		return media->currentSource().fileName();
	}

	void MediaPlayer::hasVideoChanged(bool hasVideo)
	{
		if (hasVideo)
			openVideo();
		else
			closeVideo();
	}
}
