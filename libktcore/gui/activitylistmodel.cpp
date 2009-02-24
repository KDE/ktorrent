/***************************************************************************
 *   Copyright (C) 2009 by Joris Guisson                                   *
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
#include <kicon.h>
#include "activitylistmodel.h"
#include "activitylistwidget.h"
#include <interfaces/activity.h>

namespace kt
{
	ActivityListModel::ActivityListModel(ActivityListWidget* widget) : QStringListModel(widget),widget(widget)
	{
	}
	
	ActivityListModel::~ActivityListModel()
	{
	}

	int ActivityListModel::rowCount(const QModelIndex & parent) const 
	{
		if (!parent.isValid())
			return activities.count();
		else
			return 0;
	}

	QVariant ActivityListModel::data(const QModelIndex& index, int role) const 
	{
		if (!index.isValid())
			return QVariant();
		
		Activity* act = (Activity*)index.internalPointer();
		if (!act)
			return QVariant();
		
		switch (role)
		{
			case Qt::DisplayRole:
				if (widget->displayMode() == ICONS_ONLY)
					return QVariant();
				else
					return act->name();
			case Qt::DecorationRole:
				if (widget->displayMode() == TEXT_ONLY)
					return QVariant();
				else
					return KIcon(act->icon());
			case Qt::ToolTipRole:
				return act->toolTip();
			case Qt::TextAlignmentRole:
				return Qt::AlignCenter;
			default:
				return QVariant();
		}
	}
	
	void ActivityListModel::addActivity(kt::Activity* act) 
	{
		activities.append(act);
		insertRow(activities.count() - 1);
	}

	void ActivityListModel::removeActivity(kt::Activity* act) 
	{
		int idx = activities.indexOf(act);
		if (idx < 0)
			return;
		
		activities.removeAll(act);
		removeRow(idx);
	}

	QModelIndex ActivityListModel::index(int row, int column, const QModelIndex& parent) const 
	{
		if (parent.isValid() || row < 0 || row >= activities.count())
			return QModelIndex();
		
		return createIndex(row,column,activities[row]);
	}

	QModelIndex ActivityListModel::indexOf(Activity* act) const
	{
		int r = activities.indexOf(act);
		if (r < 0)
			return QModelIndex();
		else
			return index(r,0,QModelIndex());
	}
	
	void ActivityListModel::emitLayoutChanged()
	{
		emit layoutChanged();
	}
}