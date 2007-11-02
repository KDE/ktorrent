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
#include <util/constants.h>
#include <ksharedconfig.h>

class KMenu;

namespace kt
{
	class Core;
	class ViewItem;
	class ViewMenu;
	class Group;
	class TorrentInterface;

	
	
	class View : public QTreeWidget
	{
		Q_OBJECT
	public:
		View(Core* core,QWidget* parent);
		virtual ~View();

		/**
		 * Set the group to show in this view
		 * @param g The Group
		 * */
		void setGroup(Group* g);

		/**
		 * Put the current selection in a list.
		 * @param sel The list to put it in
		 */
		void getSelection(QList<kt::TorrentInterface*> & sel);

		/// Get the current group
		const Group* getGroup() const {return group;}

		/// Save the view's state
		void saveState(KSharedConfigPtr cfg,int idx);

		/// Load the view's state
		void loadState(KSharedConfigPtr cfg,int idx);

		/// Get the current torrent
		kt::TorrentInterface* getCurrentTorrent();

		/// Get the view's caption
		QString caption() const;

		/// Check if we need to update the caption
		bool needToUpdateCaption();

	public slots:
		/**
		 * Update all items in the view
		 * @return true If the view caption nees to be updated
		 * */
		bool update();
		void addTorrent(kt::TorrentInterface* ti);
		void removeTorrent(kt::TorrentInterface* ti);
		void startTorrents();
		void stopTorrents();
		void removeTorrents();
		void removeTorrentsAndData();
		void startAllTorrents();
		void stopAllTorrents();
		void queueTorrents();
		void checkData();
		void addPeers();
		void manualAnnounce();
		void previewTorrents();
		void openDataDir();
		void openTorDir();
		void removeFromGroup();
		void speedLimitsDlg();
		void toggleDHT();
		void togglePEX();
		void scrape();
		void showMenu(const QPoint & pos);
		void showHeaderMenu(const QPoint& pos);
		void onHeaderMenuItemTriggered(QAction* act);
		void onCurrentItemChanged(QTreeWidgetItem * current,QTreeWidgetItem * previous);

	signals:
		void wantToRemove(kt::TorrentInterface* tc,bool data_to);
		void wantToStop(kt::TorrentInterface* tc,bool user);
		void wantToStart(kt::TorrentInterface* tc);
		void currentTorrentChanged(View* v,kt::TorrentInterface* tc);

	private:
		Core* core;
		Group* group;
		QMap<kt::TorrentInterface*,ViewItem*> items;
		ViewMenu* menu;
		KMenu* header_menu;
		QMap<QAction*,int> column_idx_map;
		bt::Uint32 num_torrents;
		bt::Uint32 num_running;
	};
}

#endif
