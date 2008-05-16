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

#include <QReadLocker>

#include "matches.h"

namespace kt
	{
	
	Matches::Matches()
		{
		//let's initialize the column list
		columnNames << "Name";
		}
	
	int Matches::rowCount(const QModelIndex & parent) const
		{
		Q_UNUSED(parent);
		QReadLocker readLock(&lock);
		
		return root.elementsByTagName("Match").count();
		}
	
	int Matches::columnCount(const QModelIndex & parent) const
		{
		Q_UNUSED(parent);
		QReadLocker readLock(&lock);
		
		return columnNames.count();
		}
	
	QVariant Matches::headerData(int section, Qt::Orientation orientation,int role) const
		{
		QReadLocker readLock(&lock);
		
		if (orientation==Qt::Horizontal)
			return QVariant();
		
		if (role != Qt::DisplayRole)
			return QVariant();
		
		if (section <= 0 || section >= columnNames.count())
			return QVariant();
		
		//columnNames contains the title for each column
		return columnNames.at(section);
		}
	
	QVariant Matches::data(const QModelIndex & index, int role) const
		{
		QReadLocker readLock(&lock);
		
		if (role != Qt::DisplayRole)
			return QVariant();
		
		if (index.column() < 0 || index.column() >= columnNames.count())
			return QVariant();
		
		if (index.row() < 0 || index.row() >= root.elementsByTagName("Match").count())
			return QVariant();
		
		//we know it's something within the list - so let's find the data
		QDomElement curMatch = root.elementsByTagName("Match").at(index.row()).toElement();
		QDomElement curVar;
		
		for (int i=0; i<curMatch.elementsByTagName("Variable").count(); i++)
			{
			curVar = curMatch.elementsByTagName("Variable").at(i).toElement();
			if (curVar.hasAttribute(columnNames.at(index.column())))
				return curVar.attribute(columnNames.at(index.column()));
			}
		
		return QVariant();
		}
	
	QModelIndex Matches::index(int row,int column,const QModelIndex & parent) const
		{
		Q_UNUSED(parent);
		return createIndex(row, column);
		}
	
	QModelIndex Matches::parent(const QModelIndex & index) const
		{
		Q_UNUSED(index);
		return QModelIndex();
		}
	
	}
