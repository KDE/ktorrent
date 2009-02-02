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
#include <interfaces/guiinterface.h>
#include <ksharedconfig.h>

class KMenu;

namespace kt
{
	class Core;
	class ViewModel;
	class ViewSelectionModel;
	class Group;
	class TorrentInterface;	
	
	class View : public QTreeView
	{
		Q_OBJECT
	public:
		View(Core* core,QWidget* parent);
		virtual ~View();

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

		/**
		 * Put the current selection in a list.
		 * @param sel The list to put it in
		 */
		void getSelection(QList<bt::TorrentInterface*> & sel);

		/// Get the current group
		const Group* getGroup() const {return group;}

		/// Save the view's state
		void saveState(KSharedConfigPtr cfg,int idx);

		/// Load the view's state
		void loadState(KSharedConfigPtr cfg,int idx);

		/// Get the current torrent
		bt::TorrentInterface* getCurrentTorrent();

		/// Get the view's caption
		QString caption() const;

		/// Check if we need to update the caption
		bool needToUpdateCaption();
		
		/// Get a list of column actions to plugin in the right click menu of a view
		QList<QAction*> columnActionList() const;
		
		/**
		 * Setup the default columns of the view depending on the group it is showing.
		 */
		void setupDefaultColumns();
		
		/// Get the number of torrents
		bt::Uint32 numTorrents() const {return num_torrents;};
		
		/// Get the number of running torrents
		bt::Uint32 numRunningTorrents() const {return num_running;}
		
		virtual void closeEditor(QWidget* editor,QAbstractItemDelegate::EndEditHint hint);
		virtual bool edit(const QModelIndex & index,EditTrigger trigger,QEvent* event);
	public slots:
		/**
		 * Update all items in the view
		 * */
		void update();
		void startTorrents();
		void stopTorrents();
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
		void toggleDHT();
		void togglePEX();
		void scrape();
		void moveData();
		void renameTorrent();
		void showMenu(const QPoint & pos);
		void showHeaderMenu(const QPoint& pos);
		void onHeaderMenuItemTriggered(QAction* act);
		void onCurrentItemChanged(const QModelIndex & current,const QModelIndex & previous);
		void onSelectionChanged(const QItemSelection & selected,const QItemSelection & deselected);

	signals:
		void currentTorrentChanged(View* v,bt::TorrentInterface* tc);
		void torrentSelectionChanged(View* v);
		void showMenu(View* v,const QPoint & pos);
		void editingItem(bool on);
		
	private:
		Core* core;
		Group* group;
		KMenu* header_menu;
		QMap<QAction*,int> column_idx_map;
		QList<QAction*> column_action_list;
		bt::Uint32 num_torrents;
		bt::Uint32 num_running;
		ViewModel* model;
		ViewSelectionModel* selection_model;
	};
}

#endif
