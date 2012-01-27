/***************************************************************************
 *   Copyright (C) 2012 by Joris Guisson                                   *
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
#ifndef KT_GROUPMODEL_H
#define KT_GROUPMODEL_H

#include <QAbstractListModel>


namespace kt
{
	class Group;
	class GroupManager;

	/**
		Simple list model for the view switcher combobox
	 */
	class GroupModel : public QAbstractListModel
	{
		Q_OBJECT
	public:
		GroupModel(GroupManager* gman, QObject* parent);
		virtual ~GroupModel();
		
		virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
		virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
		
		/// Get a group given the index
		Group* group(int idx) const;
		
		/// Get the index given a group
		int groupIndex(Group* g) const;
		
	private slots:
		void customGroupChanged(QString oldName, QString newName);
		void groupAdded(Group* g);
		void groupRemoved(Group* g);
		
	private:
		GroupManager* gman;
		QList<Group*> groups;
	};

}

#endif // KT_GROUPMODEL_H
