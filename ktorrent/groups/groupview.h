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
#include "groupviewmodel.h"

class KAction;
class KActionCollection;

namespace kt
{
	class GUI;
	class Core;
	class View;
	class Group;
	class GroupView;
	class GroupManager;
	class View;


	/**
		@author Joris Guisson <joris.guisson@gmail.com>
	*/
	class GroupView : public QTreeView
	{
		Q_OBJECT
	public:
		GroupView(GroupManager* gman, View* view, Core* core, GUI* gui, QWidget* parent);
		virtual ~GroupView();

		/// Save the status of the group view
		void saveState(KSharedConfigPtr cfg);

		/// Load status from config
		void loadState(KSharedConfigPtr cfg);

		/// Create a new group
		Group* addNewGroup();

		/// Setup all the actions of the GroupView
		void setupActions(KActionCollection* col);
		
	public slots:
		/// Update the group count
		void updateGroupCount();
		
	private slots:
		void onItemClicked(const QModelIndex & index);
		void showContextMenu(const QPoint & p);
		void addGroup();
		void removeGroup();
		void editGroupName();
		void editGroupPolicy();

	signals:
		void currentGroupChanged(kt::Group* g);

	private:
		virtual void keyPressEvent(QKeyEvent* event);

	private:
		GUI* gui;
		Core* core;
		View* view;
		GroupManager* gman;
 		GroupViewModel* model;

		KAction* new_group;
		KAction* edit_group;
		KAction* remove_group;
		KAction* edit_group_policy;

		friend class GroupViewItem;
	};

}

#endif
