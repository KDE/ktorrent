/***************************************************************************
 *   Copyright (C) 2008 by Alan Jones                                      *
 *   skyphyr@gmail.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
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

#include "filterlistmodel.h"

namespace kt
{

	FilterListModel::FilterListModel(CoreInterface* core, GUIInterface* gui, QObject* parent) 
			: QAbstractListModel(parent),core(core),gui(gui)
	{
	}


	FilterListModel::~FilterListModel()
	{
		qDeleteAll(filters);
	}
	
	int FilterListModel::rowCount(const QModelIndex & parent) const
	{
		if (!parent.isValid())
			return filters.count();
		else
			return 0;
	}
	
	QVariant FilterListModel::headerData(int section, Qt::Orientation orientation,int role) const
	{
		Q_UNUSED(section);
		Q_UNUSED(orientation);
		Q_UNUSED(role);
		return QVariant();
	}
	
	QVariant FilterListModel::data(const QModelIndex & index, int role) const
	{
		if (index.column() != 0)
			return QVariant();
		
		Filter* filter = (Filter*)index.internalPointer();
		if (filter)
		{
			switch (role)
			{
				case Qt::ToolTipRole:
					break;
				case Qt::DisplayRole:
					return filter->getName();
					break;
				case Qt::DecorationRole:
					return KIcon(filter->getIconName());
				//case Qt::UserRole:
				
				default:
					return QVariant();
			}
		}
		
		return QVariant();
	}
	
	QModelIndex FilterListModel::next(const QModelIndex & idx) const
	{
		int nextRow = idx.row() + 1;
		
		if (nextRow >= filters.count())
			return QModelIndex();
		
		Filter* filter = filters.at(nextRow);
		
		return createIndex(nextRow, 0, filter);
	}
	
}
