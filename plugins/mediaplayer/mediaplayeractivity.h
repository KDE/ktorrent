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

#ifndef KT_MEDIAPLAYERACTIVITY_H
#define KT_MEDIAPLAYERACTIVITY_H

#include <QModelIndex>
#include <QSplitter>
#include <KTabWidget>
#include <interfaces/activity.h>
#include <QToolButton>

class QToolBar;
class KAction;
class KActionCollection;

namespace kt 
{
	class MediaView;
	class MediaPlayer;
	class MediaModel;
	class CoreInterface;
	class VideoWidget;
	class PlayListWidget;
	
	/**
	 * Activity for the media player plugin.
	 */
	class MediaPlayerActivity : public Activity
	{
		Q_OBJECT
	public:
		MediaPlayerActivity(CoreInterface* core,QWidget* parent);
		virtual ~MediaPlayerActivity();
		
		void setupActions(KActionCollection* ac);
		void saveState(KSharedConfigPtr cfg);
		void loadState(KSharedConfigPtr cfg);
		
	public slots:
		void play();
		void play(const QString & file);
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
		void closeTab();
		void clearPlayList();
		void addMedia();
		void currentTabChanged(int idx);
		
	private:
		QSplitter* splitter;
		MediaModel* media_model;
		MediaPlayer* media_player;
		MediaView* media_view;
		KTabWidget* tabs;
		int action_flags;
		VideoWidget* video;
		bool video_shown;
		bool fullscreen_mode;
		QModelIndex curr_item;
		PlayListWidget* play_list;
		QToolButton* close_button;
		
		KAction* play_action;
		KAction* pause_action;
		KAction* stop_action;
		KAction* prev_action;
		KAction* next_action;
		KAction* show_video_action;
		KAction* clear_action;
		KAction* add_media_action;
	};

}

#endif // KT_MEDIAPLAYERACTIVITY_H
