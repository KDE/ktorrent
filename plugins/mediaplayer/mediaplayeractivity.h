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
#include <interfaces/activity.h>
#include "mediafile.h"

class QToolButton;
class QSplitter;
class QTabWidget;
class QAction;
class KActionCollection;

namespace kt
{
    class MediaView;
    class MediaPlayer;
    class MediaModel;
    class CoreInterface;
    class VideoWidget;
    class PlayListWidget;
    class MediaController;

    /**
     * Activity for the media player plugin.
     */
    class MediaPlayerActivity : public Activity
    {
        Q_OBJECT
    public:
        MediaPlayerActivity(CoreInterface* core, KActionCollection* ac, QWidget* parent);
        ~MediaPlayerActivity();

        void setupActions();
        void saveState(KSharedConfigPtr cfg);
        void loadState(KSharedConfigPtr cfg);

    public slots:
        void play();
        void play(const MediaFileRef& file);
        void pause();
        void stop();
        void prev();
        void next();
        void enableActions(unsigned int flags);
        void onSelectionChanged(const MediaFileRef& file);
        void openVideo();
        void closeVideo();
        void setVideoFullScreen(bool on);
        void onDoubleClicked(const MediaFileRef& file);
        void randomPlayActivated(bool on);
        void aboutToFinishPlaying();
        void showVideo(bool on);
        void closeTab();
        void currentTabChanged(int idx);

    private:
        QSplitter* splitter;
        MediaModel* media_model;
        MediaPlayer* media_player;
        MediaView* media_view;
        MediaController* controller;
        QTabWidget* tabs;
        int action_flags;
        VideoWidget* video;
        bool fullscreen_mode;
        QModelIndex curr_item;
        PlayListWidget* play_list;
        QToolButton* close_button;

        QAction * play_action;
        QAction * pause_action;
        QAction * stop_action;
        QAction * prev_action;
        QAction * next_action;
        QAction * show_video_action;
        QAction * clear_action;
        QAction * add_media_action;
        QAction * status_action;
        KActionCollection* ac;
    };

}

#endif // KT_MEDIAPLAYERACTIVITY_H
