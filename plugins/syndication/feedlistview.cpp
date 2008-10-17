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
#include <QVBoxLayout>
#include <ktoolbar.h>
#include <kactioncollection.h>
#include "feedlistview.h"
#include "feedlist.h"

namespace kt
{

	FeedListView::FeedListView(KActionCollection* ac,FeedList* feeds,QWidget* parent)
			: QWidget(parent),feeds(feeds)
	{
		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->setSpacing(0);
		layout->setMargin(0);
		
		tool_bar = new KToolBar(this);
		tool_bar->setToolButtonStyle(Qt::ToolButtonIconOnly);
		tool_bar->addAction(ac->action("add_feed"));
		tool_bar->addAction(ac->action("remove_feed"));
		tool_bar->addSeparator();
		tool_bar->addAction(ac->action("show_feed"));
		layout->addWidget(tool_bar);
		
		view = new QListView(this);
		view->setSelectionMode(QAbstractItemView::ContiguousSelection);
		layout->addWidget(view);
		
		view->setModel(feeds);
		connect(view,SIGNAL(doubleClicked(const QModelIndex &)),this,SLOT(itemActivated(const QModelIndex&)));
		connect(view->selectionModel(),SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
				this,SLOT(selectionChanged(const QItemSelection&, const QItemSelection&)));
	}


	FeedListView::~FeedListView()
	{
	}

	void FeedListView::itemActivated(const QModelIndex & idx)
	{
		feedActivated(feeds->feedForIndex(idx));
	}

	void FeedListView::selectionChanged(const QItemSelection& sel, const QItemSelection& desel)
	{
		Q_UNUSED(desel);
		Q_UNUSED(sel);
		enableRemove(view->selectionModel()->selectedRows().count() > 0);
	}
	
	QModelIndexList FeedListView::selectedFeeds()
	{
		return view->selectionModel()->selectedRows();
	}
}
