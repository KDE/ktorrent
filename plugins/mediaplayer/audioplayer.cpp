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
#include "audioplayer.h"

using namespace bt;

namespace kt
{

	AudioPlayer::AudioPlayer(QObject* parent)
			: QObject(parent),slider(0)
	{
		media = new Phonon::MediaObject(this);
		audio = new Phonon::AudioOutput(Phonon::MusicCategory, this);
		Phonon::createPath(media,audio);
		
		connect(media,SIGNAL(stateChanged(Phonon::State,Phonon::State)),
				this,SLOT(onStateChanged(Phonon::State, Phonon::State)));
		connect(media,SIGNAL(tick(qint64)),this,SLOT(onTimerTick(qint64)));
		media->setTickInterval(1000);
	}


	AudioPlayer::~AudioPlayer()
	{
	}

	void AudioPlayer::play(const QString & file)
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
		
	void AudioPlayer::pause()
	{
		media->pause();
	}
		
	void AudioPlayer::stop()
	{
		media->stop();
		if (slider)
		{
			slider->setValue(0);
			slider->setDisabled(true);
		}
	}
	
	void AudioPlayer::prev()
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
	
	void AudioPlayer::onStateChanged(Phonon::State cur, Phonon::State)
	{
		unsigned int flags = 0;
		switch (cur)
		{
			case Phonon::LoadingState:
				Out(SYS_GEN|LOG_DEBUG) << "AudioPlayer: loading" << endl;
				break;
			case Phonon::StoppedState:
				Out(SYS_GEN|LOG_DEBUG) << "AudioPlayer: stopped" << endl;
				flags = MEDIA_PLAY;
				if (history.count() > 0)
					flags |= MEDIA_PREV;
				
				if (slider)
				{
					slider->setDisabled(true);
					slider->setValue(0);
				}
				
				enableActions(flags);
				break;
			case Phonon::PlayingState:
				Out(SYS_GEN|LOG_DEBUG) << "AudioPlayer: playing" << endl;
				flags = MEDIA_PAUSE|MEDIA_STOP;
				if (history.count() > 1)
					flags |= MEDIA_PREV;
				
				if (slider)
				{
					slider->setEnabled(true);
					slider->setRange(0,media->totalTime());
					slider->setValue(media->currentTime());
				}
				
				enableActions(flags);
				break;
			case Phonon::BufferingState:
				Out(SYS_GEN|LOG_DEBUG) << "AudioPlayer: buffering" << endl;
				break; 
			case Phonon::PausedState:
				Out(SYS_GEN|LOG_DEBUG) << "AudioPlayer: paused" << endl;
				flags = MEDIA_PLAY|MEDIA_STOP;
				if (history.count() > 1)
					flags |= MEDIA_PREV;
				
				enableActions(flags);
				break;
			case Phonon::ErrorState:
				Out(SYS_GEN|LOG_DEBUG) << "AudioPlayer: error " << media->errorString() << endl;
				flags = MEDIA_PLAY;
				if (history.count() > 0)
					flags |= MEDIA_PREV;
				
				if (slider)
					slider->setDisabled(true);
				
				enableActions(flags);
				break;
		}
		
		
	}
	
	void AudioPlayer::onTimerTick(qint64 t)
	{
		if (slider)
			slider->setValue(t);
	}
	
	void AudioPlayer::setSlider(QSlider* s)
	{
		if (slider)
			disconnect(slider,SIGNAL(sliderMoved(int)),this,SLOT(seek(int)));
		
		slider = s;
		if (slider)
		{
			connect(slider,SIGNAL(sliderMoved(int)),this,SLOT(seek(int)));
			slider->setDisabled(true);
			if (media->state() == Phonon::PlayingState)
			{
				slider->setEnabled(true);
				slider->setRange(0,media->totalTime());
				slider->setValue(media->currentTime());
			}
		}
	}
	
	void AudioPlayer::seek(int val)
	{
		if (media->state() != Phonon::PlayingState || !media->isSeekable())
			return;
		
		media->seek(val);
	}
	
	QString AudioPlayer::getCurrentSource() const
	{
		return media->currentSource().fileName();
	}

}
