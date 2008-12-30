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
#include "filterlistview.h"
#include "filterlist.h"

namespace kt
{

	FilterListView::FilterListView(FilterList* filters,QWidget* parent)
			: QListView(parent),filters(filters)
	{
		setContextMenuPolicy(Qt::CustomContextMenu);
		setModel(filters);
		setAlternatingRowColors(true);
		connect(this,SIGNAL(doubleClicked(const QModelIndex &)),this,SLOT(itemActivated(const QModelIndex&)));
		connect(this->selectionModel(),SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
				this,SLOT(selectionChanged(const QItemSelection&, const QItemSelection&)));
	}


	FilterListView::~FilterListView()
	{
	}

	void FilterListView::itemActivated(const QModelIndex & idx)
	{
		filterActivated(filters->filterForIndex(idx));
	}

	void FilterListView::selectionChanged(const QItemSelection& sel, const QItemSelection& desel)
	{
		Q_UNUSED(desel);
		Q_UNUSED(sel);
		enableRemove(selectionModel()->selectedRows().count() > 0);
		enableEdit(selectionModel()->selectedRows().count() == 1);
	}
	
	QModelIndexList FilterListView::selectedFilters()
	{
		return selectionModel()->selectedRows();
	}
}
