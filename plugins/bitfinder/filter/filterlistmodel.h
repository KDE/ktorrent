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

#ifndef KTFILTERLISTMODEL_H
#define KTFILTERLISTMODEL_H

#include <QAbstractItemModel>
#include <util/constants.h>

#include "filter.h"

namespace kt
{
	class CoreInterface;
	class GUIInterface;

	class FilterListModel : public QAbstractListModel
	{
		Q_OBJECT
	public:
		FilterListModel(CoreInterface* core, GUIInterface* gui, QObject* parent);
		virtual ~FilterListModel();
		
		virtual int rowCount(const QModelIndex & parent) const;
		virtual QVariant headerData(int section, Qt::Orientation orientation,int role) const;
		virtual QVariant data(const QModelIndex & index, int role) const;
		
 		QModelIndex next(const QModelIndex & idx) const;
		
	public slots:
		
	private:
		CoreInterface* core;
		GUIInterface* gui;
		QList<Filter*> filters;
	};

}

#endif
