/***************************************************************************
 *   Copyright (C) 2012 by                                                 *
 *   Joris Guisson <joris.guisson@gmail.com>                               *
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

#ifndef KT_GROUPVIEWMODEL_H
#define KT_GROUPVIEWMODEL_H

#include <QAbstractItemModel>
#include <QStringList>


namespace kt
{

	class GroupView;

	class View;
	class Group;
	class GroupManager;

	/**
	 * Model for the GroupView
	 **/
	class GroupViewModel : public QAbstractItemModel
	{
		Q_OBJECT
	public:
		GroupViewModel(GroupManager* gman, View* view, QObject* parent);
		virtual ~GroupViewModel();
		
		virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
		virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
		virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
		virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
		virtual QModelIndex parent(const QModelIndex& child) const;
		virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
		virtual bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent);
		virtual Qt::DropActions supportedDropActions() const;
		virtual Qt::ItemFlags flags(const QModelIndex& index) const;
		virtual QStringList mimeTypes() const;
		virtual bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());
		
		/// Get the group given an index
		Group* groupForIndex(const QModelIndex & index) const;
		
		/// Get all the expanded groups
		QStringList expandedGroups(GroupView* gview);
		
		/// Expand all items in the tree which are in the groups list
		void expandGroups(GroupView* gview, const QStringList & groups);
		
		/// Update the group count
		void updateGroupCount(const QModelIndex & idx);
		
	private slots:
		void groupAdded(Group* g);
		void groupRemoved(Group* g);
		
	private:
		struct Item
		{
			Item(const QString & name, Item* parent, int row, GroupViewModel* model);
			
			void insert(const QString & name, const QString & p, const QModelIndex & idx);
			void insert(Group* g, const QModelIndex & idx);
			void remove(Group* g, const QModelIndex & idx);
			bool operator == (const QString & n) const;
			QVariant displayData();
			QVariant decoration();
			void expandedGroups(GroupView* gview, QStringList & groups, const QModelIndex & idx) const;
			void expandGroups(GroupView* gview, const QStringList & groups, const QModelIndex & idx);
			QString path() const;
			void dump();
			QModelIndex findGroup(Group* g, const QModelIndex & idx);
			
			QString name;
			QString display_name;
			Item* parent;
			int row;
			Group* group;
			QList<Item> children;
			GroupViewModel* model;
		};
		
		QModelIndex findGroup(Group* g);
		
	private:
		Item root;
		GroupManager* gman;
		View* view;
	};

}

#endif // KT_GROUPVIEWMODEL_H
