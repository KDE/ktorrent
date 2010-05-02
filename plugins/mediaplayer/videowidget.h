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
#ifndef KTVIDEOWIDGET_H
#define KTVIDEOWIDGET_H

#include <QWidget>
#include <Phonon/VideoWidget>
#include <Phonon/MediaObject>
#include <Phonon/SeekSlider>
#include <Phonon/VolumeSlider>

class QAction;
class KToolBar;

namespace kt
{
	class MediaPlayer;

	/**
	 * Widget to display a video
	 * @author Joris Guisson
	*/
	class VideoWidget : public QWidget
	{
		Q_OBJECT
	public:
		VideoWidget(MediaPlayer* player,QWidget* parent);
		virtual ~VideoWidget();
		
		virtual void mouseMoveEvent(QMouseEvent* event);
		
		/**
		 * Make the widget full screen or not.
		 * @param on 
		 */
		void setFullScreen(bool on);
		
	private slots:
		void play();
		void pause();
		void stop();
		void setControlsVisible(bool on);
		void onStateChanged(Phonon::State cur,Phonon::State old);
		
	signals:
		void toggleFullScreen(bool on);
		
	private:
		void inhibitScreenSaver(bool on);

	private:
		Phonon::VideoWidget* video;
		MediaPlayer* player;
		Phonon::SeekSlider* slider;
		KToolBar* tb;
		QAction* play_act;
		QAction* pause_act;
		QAction* stop_act;
		Phonon::VolumeSlider* volume;
		bool fullscreen;
		uint screensaver_cookie;
		int powermanagement_cookie;
	};

}

#endif
