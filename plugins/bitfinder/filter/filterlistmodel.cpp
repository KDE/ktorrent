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

	FilterListModel::FilterListModel(CoreInterface* core,QObject* parent) : QAbstractItemModel(parent),core(core)
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
	
	int FilterListModel::columnCount(const QModelIndex & parent) const
	{
		Q_UNUSED(parent);
		return 1;
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
					return KIcon("view-filter");
				//case Qt::UserRole:
				
				default:
					return QVariant();
			}
		}
		
		return QVariant();
	}
	
	bool FilterListModel::removeRows(int row,int count,const QModelIndex & parent)
	{
		if (parent.isValid())
			return false;
		
		beginRemoveRows(QModelIndex(),row,row + count - 1);
		for (int i = 0;i < count;i++)
		{
			if (row >= 0 && row < filters.count())
			{
				Filter* filter = filters[row];
				filters.removeAt(row);
				delete filter;
			}
		}
		endRemoveRows();
		return true;
	}
	
	bool FilterListModel::insertRows(int row,int count,const QModelIndex & parent)
	{
		if (parent.isValid())
			return false;
					
		beginInsertRows(QModelIndex(),row,row + count - 1);
		endInsertRows();
		return true;
	}
	
	QModelIndex FilterListModel::index(int row,int column,const QModelIndex & parent) const
	{
		if (column != 0)
			return QModelIndex();
		
		if (!parent.isValid() && row >= 0 && row < filters.count())
		{
			Filter* filter = filters.at(row);
			return createIndex(row,column,filter); // it's a torrent
		}
		
		return QModelIndex();
	}
	
	QModelIndex FilterListModel::next(const QModelIndex & idx) const
	{
		int nextRow = idx.row() + 1;
		
		if (nextRow >= filters.count())
			return QModelIndex();
		
		Filter* filter = filters.at(nextRow);
		
		return createIndex(nextRow, 0, filter);
	}
	
	QModelIndex FilterListModel::parent(const QModelIndex &child) const
		{
		Q_UNUSED(child);
		
		return QModelIndex();
		}
	
}
