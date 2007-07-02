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
#ifndef KTGROUPVIEW_H
#define KTGROUPVIEW_H

#include <klistview.h>
#include <util/ptrmap.h>

class KPopupMenu;
class KActionCollection;
class ViewManager;
class KTorrentView;

namespace kt
{
	class Group;
	class GroupView;
	class GroupManager;
	class TorrentInterface;
	
	class GroupViewItem : public KListViewItem
	{
		Group* g;
		GroupView* gview;
	public:
		GroupViewItem(GroupView* parent,Group* g);
		GroupViewItem(GroupView* gview,KListViewItem* parent,Group* g);
		virtual ~GroupViewItem();
		
		virtual int compare(QListViewItem* i,int col,bool ascending) const; 
	};

	/**
		@author Joris Guisson <joris.guisson@gmail.com>
	*/
	class GroupView : public KListView
	{
		Q_OBJECT
	public:
		GroupView(ViewManager* view,KActionCollection* col,QWidget *parent = 0, const char *name = 0);
		virtual ~GroupView();
		
		/// Get the current group
		Group* currentGroup() {return current;} 
		
		/// Save all groups
		void saveGroups();
		
		/// Load groups
		void loadGroups();
		
		/// Find a group by its name
		const Group* findGroup(const QString & name) const;
		
		GroupManager* groupManager() const { return gman; }
		
	public slots:
		void onTorrentRemoved(kt::TorrentInterface* tc);
		
		/// Update a groups sub menu
		void updateGroupsSubMenu(KPopupMenu* gsm);
		
		/// An item was activated in the groups sub menu of a KTorrentView
		void onGroupsSubMenuItemActivated(KTorrentView* v,const QString & group);
		
	private slots:
		void onExecuted(QListViewItem* item);
		void showContextMenu(KListView* ,QListViewItem* item,const QPoint & p);
		void addGroup();
		void removeGroup();
		void editGroupName();
		void onDropped(QDropEvent* e,QListViewItem *after);
		virtual bool acceptDrag(QDropEvent* event) const;
		void openView();
		
		
	signals:
		void currentGroupChanged(kt::Group* g);
		void groupRenamed(kt::Group* g);
		void openNewTab(kt::Group* g);
		void groupRemoved(kt::Group* g);
		
	private:
		void createMenu(KActionCollection* col);
		GroupViewItem* addGroup(Group* g,KListViewItem* parent);
			
	private:
		ViewManager* view;
		KListViewItem* custom_root;
		bt::PtrMap<GroupViewItem*,Group> groups;
		GroupManager* gman;
		QString save_file;
		
		Group* current;
		
		GroupViewItem* current_item;
		KPopupMenu* menu;
		KAction* new_group;
		KAction* edit_group;
		KAction* remove_group;
		KAction* open_in_new_tab;
		
		friend class GroupViewItem;
	};

}

#endif
