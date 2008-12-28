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
#ifndef KTMEDIAPLAYERPLUGIN_H
#define KTMEDIAPLAYERPLUGIN_H

#include <QModelIndex>
#include <interfaces/plugin.h>
#include <interfaces/guiinterface.h>

class KAction;


namespace kt
{
	class MediaView;
	class MediaModel;
	class MediaPlayer;
	class VideoWidget;

	/**
		@author
	*/
	class MediaPlayerPlugin : public Plugin,public CloseTabListener
	{
		Q_OBJECT
	public:
		MediaPlayerPlugin(QObject* parent, const QStringList& args);
		virtual ~MediaPlayerPlugin();

		virtual void load();
		virtual void unload();
		virtual bool versionCheck(const QString& version) const;
	private:
		void setupActions();
		virtual void tabCloseRequest(kt::GUIInterface* gui,QWidget* tab);
		
	private slots:
		void play();
		void pause();
		void stop();
		void prev();
		void next();
		void enableActions(unsigned int flags);
		void onSelectionChanged(const QModelIndex & idx);
		void openVideo();
		void closeVideo();
		void setVideoFullScreen(bool on);
		void onDoubleClicked(const QModelIndex & idx);
		void randomPlayActivated();
		void aboutToFinishPlaying();
		void showVideo(bool on);
		
	private:
		MediaView* media_view;
		MediaModel* media_model;
		MediaPlayer* media_player;
		KAction* play_action;
		KAction* pause_action;
		KAction* stop_action;
		KAction* prev_action;
		KAction* next_action;
		KAction* show_video_action;
		int action_flags;
		VideoWidget* video;
		bool video_shown;
		bool fullscreen_mode;
		QDialog* fs_dialog;
		QModelIndex curr_item;
	};

}

#endif
