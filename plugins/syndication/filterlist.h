/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
#ifndef KTFILTERLIST_H
#define KTFILTERLIST_H

#include <QAbstractListModel>

namespace kt
{
	class Filter;

	/**
		Model to keep track of all filters
	*/
	class FilterList : public QAbstractListModel
	{
		Q_OBJECT
	public:
		FilterList(QObject* parent);
		virtual ~FilterList();
		
		virtual int rowCount(const QModelIndex & parent) const;
		virtual QVariant data(const QModelIndex & index, int role) const;
		virtual bool removeRows(int row,int count,const QModelIndex & parent);
		virtual bool insertRows(int row,int count,const QModelIndex & parent);
		
		void addFilter(Filter* f);
		void removeFilter(Filter* f);
		Filter* filterForIndex(const QModelIndex & idx);
		
		void saveFilters(const QString & file);
		void loadFilters(const QString & file);
		void filterEdited(Filter* filter);
	private:
		QList<Filter*> filters;
	};

}

#endif
