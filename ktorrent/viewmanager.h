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
#ifndef KT_VIEWMANAGER_HH
#define KT_VIEWMANAGER_HH

#include <QList>
#include <interfaces/guiinterface.h>
#include <ksharedconfig.h>

#include <torrent/queuemanager.h>

class QWidget;


namespace kt
{
	class View;
	class Core;
	class GUI;
	class Group;

	class ViewManager : public QObject,public CloseTabListener
	{
		Q_OBJECT
	public:
		ViewManager(Group* all_group,GUI* gui,Core* core);
		virtual ~ViewManager();
		
		/// Create a new view
		View* newView(Core* core,QWidget* parent);
		
		/// Save all views
		void saveState(KSharedConfigPtr cfg);
		
		/// Restore all views from configuration
		void loadState(KSharedConfigPtr cfg);
		
		/// Update the current view
		void update();
		
		/**
		 * Put the current selection of the current view in a list.
		 * @param sel The list to put it in
		 */
		void getSelection(QList<bt::TorrentInterface*> & sel);
		
		/// Get the current view
		View* getCurrentView() {return current;}

		/// Get the current torrent
		const bt::TorrentInterface* getCurrentTorrent() const;
		
		/// Get the current torrent (non const version)
		bt::TorrentInterface* getCurrentTorrent();
		
		/// Setup the actions of the view manager
		void setupActions();
		
		/// Update enabled or disabled state of all actions
		void updateActions();

	public slots:
		void onCurrentTabChanged(QWidget* tab);
		void onCurrentGroupChanged(kt::Group* g);
		void onGroupRenamed(kt::Group* g);
		void onGroupRemoved(kt::Group* g);
		void onGroupAdded(kt::Group* g);
		void onCurrentTorrentChanged(View* v,bt::TorrentInterface* tc);
		void onSelectionChaged(View* v);
		
		/// Start all selected downloads in the current view
		void startTorrents();
		
		/// Stop all selected downloads in the current view
		void stopTorrents();
		
		/// Start all downloads in the current view
		void startAllTorrents();
		
		/// Stop all downloads in the current view
		void stopAllTorrents();
		
		/// Enqueue or dequeue torrents
		void queueTorrents();

		/// Check the data of the selected torrent
		void checkData();

		/// Remove selected downloads in the current view
		void removeTorrents();
		
		/// Remove selected torrent
		void renameTorrent();
		
		/// Select all torrents in the current view
		void selectAll();
		
	private slots:
		/// Remove selected downloads and data in the current view
		void removeTorrentsAndData();
		
		/// Show the add peers dialog for the selected torrent
		void addPeers();

		/// Toggle DHT on and off for the selected torrents
		void toggleDHT();
		
		/// Toggle PEX for the selected torrents
		void togglePEX();
		
		/// Do a manual announce for the selected torrents
		void manualAnnounce();
		
		/// Scrape the selected torrents
		void scrape();
		
		/// Preview the selected torrents
		void previewTorrents();
		
		/// Open the data dir of selected torrents
		void openDataDir();
		
		/// Open the tor dir of selected torrents
		void openTorDir();
		
		/// Move data of the selected torrent
		void moveData();
		
		/// Remove the selected torrent from the current group
		void removeFromGroup();
		
		/// An item in the groups menu was triggered
		void addToGroupItemTriggered();
		
		/// Add a new group and add the current selection to it
		void addToNewGroup();
		
		/// Show the view menu
		void showViewMenu(View* v,const QPoint & pos);
		
		/// Copy the torrent URL to the clipboard
		void copyTorrentURL();
		
	private:
		virtual bool closeAllowed(QWidget* w);
		virtual void tabCloseRequest(kt::GUIInterface* gui,QWidget* tab);

	private:
		GUI* gui;
		Core* core;
		View* current;
		QList<View*> views;
		Group* all_group; 
		
		// actions for the view menu 
		QAction* start_torrent;
		QAction* start_all;
		QAction* stop_torrent;
		QAction* stop_all;
		QAction* remove_torrent;
		KAction* remove_torrent_and_data;
		QAction* queue_torrent;
		QAction* add_peers;
		QAction* dht_enabled;
		QAction* pex_enabled;
		KAction* manual_announce;
		QAction* do_scrape;
		QAction* preview;
		QAction* data_dir;
		QAction* tor_dir;
		QAction* move_data;
		QAction* rename_torrent;
		QAction* remove_from_group;
		QMap<Group*,QAction*> group_actions;
		QAction* add_to_new_group;
		QAction* check_data;
		QAction* open_dir_menu;
		QAction* groups_menu;
		QAction* copy_url;
		QList<QAction*> configure_columns_list;
	};
}

#endif
