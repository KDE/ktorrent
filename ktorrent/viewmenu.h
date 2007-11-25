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
#ifndef KTVIEWMENU_HH
#define KTVIEWMENU_HH

#include <kmenu.h>

class QAction;

namespace kt
{
	class View;
	class GroupManager;

	/**
	 * The menu shown in a view
	 * */
	class ViewMenu : public KMenu
	{
		Q_OBJECT
	public:
		ViewMenu(GroupManager* gman,View* parent);
		virtual ~ViewMenu();
		
		void show(const QPoint & pos);
	private:
		void updateGroupsMenu();

	private slots:
		void groupsMenuItemTriggered(QAction* act);

	private:
		View* view;
		GroupManager* gman;
		QAction* start_torrent;
		QAction* stop_torrent;
		QAction* remove_torrent;
		QAction* remove_torrent_and_data;
		QAction* queue_torrent;
		QAction* add_peers;
		QMenu* peer_sources_menu;
		QAction* dht_enabled;
		QAction* pex_enabled;
		QAction* manual_announce;
		QAction* scrape;
		QAction* preview;
		QMenu* dirs_menu;
		QAction* data_dir;
		QAction* tor_dir;
		QAction* move_data;
		QAction* remove_from_group;
		QMenu* add_to_group;
		QAction* check_data;
		QAction* speed_limits;
	};
}

#endif
