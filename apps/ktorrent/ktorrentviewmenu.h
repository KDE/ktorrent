/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
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
#ifndef KTORRENTVIEWMENU_H
#define KTORRENTVIEWMENU_H

#include <kpopupmenu.h>
		
class KTorrentView;

/**
	@author Joris Guisson <joris.guisson@gmail.com>
*/
class KTorrentViewMenu : public KPopupMenu
{
	Q_OBJECT
public:
	KTorrentViewMenu(KTorrentView *parent, const char *name = 0 );
	virtual ~KTorrentViewMenu();
	
	/// Show the menu at the given point
	void show(const QPoint & p);
	
	/// Get the group sub menu
	KPopupMenu* getGroupsSubMenu() {return groups_sub_menu;}
	
public slots:
	void gsmItemActived(int id);
	
signals:
	/// A item in the groups sub menu has been activated
	void groupItemActivated(const QString & group);

private:
	KTorrentView* view;
	KPopupMenu* groups_sub_menu;
	KPopupMenu* dirs_sub_menu;
	KPopupMenu* peer_sources_menu;
	int stop_id;
	int start_id;
	int remove_id;
	int remove_all_id;
	int preview_id;
	int announce_id;
	int queue_id;
	int scan_id;
	int remove_from_group_id;
	int add_to_group_id;
	int add_peer_id;
	int dirs_id;
	int outputdir_id;
	int torxdir_id;
	int downloaddir_id;
	int peer_sources_id;
	int dht_id;
	int ut_pex_id;
	int traffic_lim_id;
};

#endif
