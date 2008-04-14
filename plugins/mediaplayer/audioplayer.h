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
#ifndef KTAUDIOPLAYER_H
#define KTAUDIOPLAYER_H

#include <QObject>
#include <QSlider>
#include <Phonon/MediaObject>

namespace Phonon
{
	class AudioOutput;;
}


namespace kt
{
	enum ActionFlags
	{
		MEDIA_PLAY = 1,MEDIA_PAUSE = 2,MEDIA_STOP = 4
	};

	/**
		@author
	*/
	class AudioPlayer : public QObject
	{
		Q_OBJECT
	public:
		AudioPlayer(QObject* parent);
		virtual ~AudioPlayer();
		
		Phonon::AudioOutput* output() {return audio;}
		
		void setSlider(QSlider* s);
		
		/// Play a file
		void play(const QString & file);
		
		/// Pause playing
		void pause();
		
		/// Stop playing
		void stop();
	private slots:
		void onStateChanged(Phonon::State cur,Phonon::State old);
		void onTimerTick(qint64 t);
		void seek(int val);
		
	signals:
		void enableActions(int flags);

	private:
		Phonon::MediaObject* media;
		Phonon::AudioOutput* audio;
		QSlider* slider;
	};

}

#endif
