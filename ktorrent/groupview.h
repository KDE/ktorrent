/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson                                   *
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

#include <QTreeWidget>
#include <ksharedconfig.h>

class KMenu;


namespace kt
{
	class View;
	class Group;
	class GroupView;
	class GroupManager;
	class ViewManager;
		
	class GroupViewItem : public QTreeWidgetItem
	{
		Group* g;
		// this is not the group name, but the name of the group in the path (if the path is /all/foo/bar, this will be bar)
		QString path_name;  
	public: 
		GroupViewItem(GroupView* parent,Group* g,const QString & name);
		GroupViewItem(QTreeWidgetItem* parent,Group* g,const QString & name);
		virtual ~GroupViewItem();
		
		QString name() const {return path_name;}
		Group* group() {return g;}
		
		void setGroup(Group* g);
	//	virtual int compare(QListViewItem* i,int col,bool ascending) const; 
	};

	/**
		@author Joris Guisson <joris.guisson@gmail.com>
	*/
	class GroupView : public QTreeWidget
	{
		Q_OBJECT
	public:
		GroupView(GroupManager* gman,ViewManager* view,GUI* gui);
		virtual ~GroupView();
		
		/// Get the current group
		Group* currentGroup() {return current;} 

		/// Save the status of the group view
		void saveState(KSharedConfigPtr cfg);

		/// Load status from config
		void loadState(KSharedConfigPtr cfg);
		
		/// Create a new group
		Group* addNewGroup();
		
	private slots:
		void onItemActivated(QTreeWidgetItem* item,int col);
		void onItemChanged(QTreeWidgetItem* item,int col);
		void showContextMenu(const QPoint & p);
		void addGroup();
		void removeGroup();
		void editGroupName();
		void openView();
		void editGroupPolicy();
		void defaultGroupAdded(Group* g);
		void defaultGroupRemoved(Group* g);
		void customGroupAdded(Group* g);
		void customGroupRemoved(Group* g);
		
		
	signals:
		void currentGroupChanged(kt::Group* g);
		void groupRenamed(kt::Group* g);
		void openNewTab(kt::Group* g);
		
	private:
		void setupActions(KActionCollection* col);
		GroupViewItem* addGroup(Group* g,QTreeWidgetItem* parent,const QString & name);
		GroupViewItem* add(QTreeWidgetItem* parent,const QString & path,Group* g);
		void remove(QTreeWidgetItem* parent,const QString & path,Group* g);
		virtual bool dropMimeData(QTreeWidgetItem *parent, int index, 
					  const QMimeData *data,Qt::DropAction action);    
		virtual QStringList mimeTypes() const;
		virtual Qt::DropActions supportedDropActions () const;

	private:
		GUI* gui;
		ViewManager* view;
		QTreeWidgetItem* custom_root;
		GroupManager* gman;
		
		Group* current;
		GroupViewItem* current_item;

		KMenu* menu;
		KAction* new_group;
		KAction* edit_group;
		KAction* remove_group;
		KAction* open_in_new_tab;
		KAction* edit_group_policy;
		
		bool editing_item;
		
		friend class GroupViewItem;
	};

}

#endif
