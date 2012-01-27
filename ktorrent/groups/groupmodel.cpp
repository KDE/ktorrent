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
#include "groupmodel.h"

#include <KIcon>
#include "groups/groupmanager.h"


namespace kt
{
	struct GroupPathCompare
	{
		
		
		bool operator () (Group* a, Group* b)
		{
			if (a->isStandardGroup() && !b->isStandardGroup())
				return true;
			else if (!a->isStandardGroup() && b->isStandardGroup())
				return false;
			else
				return  a->groupPath() < b->groupPath();
		}
	};
	
	GroupModel::GroupModel(GroupManager* gman,QObject* parent): QAbstractListModel(parent)
	{
		connect(gman, SIGNAL(groupAdded(Group*)), this, SLOT(groupAdded(Group*)));
		connect(gman, SIGNAL(groupRemoved(Group*)), this, SLOT(groupRemoved(Group*)));
		connect(gman, SIGNAL(customGroupChanged(QString,QString)), this, SLOT(customGroupChanged(QString,QString)));
		
		for (GroupManager::Itr i = gman->begin(); i != gman->end(); i++)
		{
			groups.append(i->second);
		}
		
		qSort(groups.begin(), groups.end(), GroupPathCompare());
	}

	GroupModel::~GroupModel()
	{
	}
	

	
	QVariant GroupModel::data(const QModelIndex& index, int role) const
	{
		if (index.row() < 0 || index.row() >= groups.count())
			return QVariant();
		
		Group* g = groups.at(index.row());
		switch (role)
		{
			case Qt::DisplayRole:
				return g->groupName();
			case Qt::DecorationRole:
				return KIcon(g->groupIconName());
			default:
				return QVariant();
		}
	}
		
	int GroupModel::rowCount(const QModelIndex& parent) const
	{
		if (!parent.isValid())
			return groups.count();
		else
			return 0;
	}
	
	void GroupModel::groupAdded(Group* g)
	{
		groups.append(g);
		qSort(groups.begin(), groups.end(), GroupPathCompare());
		insertRow(groups.count(), QModelIndex());
	}

	void GroupModel::groupRemoved(Group* g)
	{
		int pos = groups.indexOf(g);
		if (pos != -1)
		{
			groups.removeAt(pos);
			removeRow(pos);
		}
	}
	
	void GroupModel::customGroupChanged(QString oldName, QString newName)
	{
		Q_UNUSED(oldName);
		
		int idx = 0;
		foreach (Group* g, groups)
		{
			if (g->groupName() == newName)
			{
				dataChanged(index(idx,0), index(idx, 0));
				break;
			}
			idx++;
		}
	}
	
	Group* GroupModel::group(int idx) const
	{
		if (idx >= 0 && idx < groups.size())
			return groups.at(idx);
		else
			return 0;
	}
	
	int GroupModel::groupIndex(Group* g) const
	{
		return groups.indexOf(g);
	}


}


