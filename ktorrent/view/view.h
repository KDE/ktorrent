/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
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

#ifndef KTVIEW_HH
#define KTVIEW_HH

#include <QMap>
#include <QTreeWidget>

#include <KSharedConfig>

#include <util/constants.h>
#include <interfaces/guiinterface.h>

class QMenu;
class QAction;
class KActionCollection;

namespace kt
{
    class GUI;
    class Extender;
    class Core;
    class ViewModel;
    class ViewSelectionModel;
    class ViewDelegate;
    class Group;

    class View : public QTreeView
    {
        Q_OBJECT
    public:
        View(Core* core, GUI* gui, QWidget* parent);
        ~View();

        /// Setup the actions of the view manager
        void setupActions(KActionCollection* ac);

        /// Update all actions
        void updateActions();

        /**
         * Get the view model
         * @return The ViewModel of this View
         */
        ViewModel* viewModel() {return model;}

        /**
         * Set the group to show in this view
         * @param g The Group
         * */
        void setGroup(Group* g);

        /// Get the current group
        Group* getCurrentGroup() const {return group;}

        /**
         * Put the current selection in a list.
         * @param sel The list to put it in
         */
        void getSelection(QList<bt::TorrentInterface*> & sel);

        /// Get the current group
        const Group* getGroup() const {return group;}

        /// Restore the view state
        void restoreState(const QByteArray& state);

        /// Get the current torrent
        bt::TorrentInterface* getCurrentTorrent();

        /// Get the number of torrents
        bt::Uint32 numTorrents() const {return num_torrents;};

        /// Get the number of running torrents
        bt::Uint32 numRunningTorrents() const {return num_running;}

        void closeEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint) override;
        bool edit(const QModelIndex& index, EditTrigger trigger, QEvent* event) override;

        /// Get the ViewDelegate
        ViewDelegate* viewDelegate() {return delegate;}

        /// Extend a widget
        void extend(bt::TorrentInterface* tc, Extender* widget, bool close_similar);

        /// Get the default state
        const QByteArray& defaultState() const {return default_state;}

        void keyPressEvent(QKeyEvent* event) override;

    public slots:
        /// Set the filter string
        void setFilterString(const QString& filter);

        /// Update all items in the view
        void update();

        void startTorrents();
        void forceStartTorrents();
        void stopTorrents();
        void pauseTorrents();
        void removeTorrents();
        void removeTorrentsAndData();
        void startAllTorrents();
        void stopAllTorrents();
        void checkData();
        void addPeers();
        void manualAnnounce();
        void previewTorrents();
        void openDataDir();
        void openTorDir();
        void removeFromGroup();
        void scrape();
        void moveData();
        void showProperties();
        void renameTorrent();
        void showMenu(const QPoint& pos);
        void showHeaderMenu(const QPoint& pos);
        void onHeaderMenuItemTriggered(QAction* act);
        void onCurrentItemChanged(const QModelIndex& current, const QModelIndex& previous);
        void onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
        void onDoubleClicked(const QModelIndex& index);
        void onCurrentGroupChanged(kt::Group* g);
        void onGroupRenamed(Group* g);
        void onGroupRemoved(Group* g);
        void onGroupAdded(Group* g);

        /// An item in the groups menu was triggered
        void addToGroupItemTriggered();

        /// Copy the torrent URL to the clipboard
        void copyTorrentURL();

        /// Show the speed limits dialog
        void speedLimits();

        /// Export a torrent
        void exportTorrent();

        /// Add a new group and add the current selection to it
        void addToNewGroup();


    signals:
        void currentTorrentChanged(bt::TorrentInterface* tc);
        void torrentSelectionChanged();
        void editingItem(bool on);

    private:
        Core* core;
        GUI* gui;
        Group* group;
        QMenu* header_menu;
        QMap<QAction*, int> column_idx_map;
        QList<QAction*> column_action_list;
        bt::Uint32 num_torrents;
        bt::Uint32 num_running;
        ViewModel* model;
        ViewSelectionModel* selection_model;
        ViewDelegate* delegate;
        QMap<bt::TorrentInterface*, Extender*> data_scan_extenders;
        QByteArray default_state;

        // actions for the view menu
        QAction * start_torrent;
        QAction * force_start_torrent;
        QAction * start_all;
        QAction * stop_torrent;
        QAction * stop_all;
        QAction * pause_torrent;
        QAction * unpause_torrent;
        QAction * remove_torrent;
        QAction * remove_torrent_and_data;
        QAction * add_peers;
        QAction * manual_announce;
        QAction * do_scrape;
        QAction * preview;
        QAction * data_dir;
        QAction * tor_dir;
        QAction * move_data;
        QAction * torrent_properties;
        QAction * rename_torrent;
        QAction * remove_from_group;
        QMap<Group*, QAction*> group_actions;
        QAction * add_to_new_group;
        QAction * check_data;
        QAction * open_dir_menu;
        QAction * groups_menu;
        QAction * copy_url;
        QAction * export_torrent;
        QList<QAction *> configure_columns_list;
        QAction * speed_limits;
    };
}

#endif
