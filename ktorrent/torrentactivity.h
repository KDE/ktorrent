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

#ifndef TORRENTACTIVITY_H
#define TORRENTACTIVITY_H

#include <QSplitter>
#include <interfaces/torrentactivityinterface.h>

class QAction;
class KToggleAction;
class KComboBox;

namespace kt
{
    class MagnetView;
    class GUI;
    class Core;
    class View;
    class GroupView;
    class QueueManagerWidget;
    class TabBarWidget;
    class Group;
    class TorrentSearchBar;
    class GroupSwitcher;

    /**
     * Activity which manages torrents.
     */
    class TorrentActivity : public TorrentActivityInterface
    {
        Q_OBJECT
    public:
        TorrentActivity(Core* core, GUI* gui, QWidget* parent);
        ~TorrentActivity();

        /// Get the group view
        GroupView* getGroupView() {return group_view;}

        void loadState(KSharedConfigPtr cfg);
        void saveState(KSharedConfigPtr cfg);
        const bt::TorrentInterface* getCurrentTorrent() const  override;
        bt::TorrentInterface* getCurrentTorrent() override;
        void updateActions() override;
        void addToolWidget(QWidget* widget, const QString& text, const QString& icon, const QString& tooltip) override;
        void removeToolWidget(QWidget* widget) override;
        Group* addNewGroup() override;

        /// Update the activity
        void update();

        /// Setup all actions
        void setupActions();

    public slots:
        /**
        * Called by the ViewManager when the current torrent has changed
        * @param tc The torrent
        * */
        void currentTorrentChanged(bt::TorrentInterface* tc);

        /**
            Hide or show the group view
        */
        void setGroupViewVisible(bool visible);

        /**
         * The suspended state has changed
         * @param suspended
         */
        void onSuspendedStateChanged(bool suspended);


    private slots:
        void startAllTorrents();
        void stopAllTorrents();
        void suspendQueue(bool suspend);
        void queueOrdered();

    private:
        Core* core;
        GUI* gui;
        View* view;
        GroupView* group_view;
        GroupSwitcher* group_switcher;
        QueueManagerWidget* qm;
        QSplitter* hsplit;
        QSplitter* vsplit;
        TabBarWidget* tool_views;
        MagnetView* magnet_view;
        TorrentSearchBar* search_bar;

        QAction * start_all_action;
        QAction * stop_all_action;
        KToggleAction* queue_suspend_action;
        QAction * show_group_view_action;
        QAction * filter_torrent_action;
    };
}

#endif // TORRENTACTIVITY_H
