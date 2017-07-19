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

#include "feedlistview.h"
#include "feedlist.h"
#include "feedlistdelegate.h"

namespace kt
{

    FeedListView::FeedListView(FeedList* feeds, QWidget* parent)
        : QListView(parent), feeds(feeds)
    {
        setContextMenuPolicy(Qt::CustomContextMenu);
        setSelectionMode(QAbstractItemView::ExtendedSelection);
        setModel(feeds);
        setItemDelegate(new FeedListDelegate(this));
        setAlternatingRowColors(true);
        setEditTriggers(QAbstractItemView::SelectedClicked | QAbstractItemView::EditKeyPressed);
        connect(this, &FeedListView::doubleClicked, this, &FeedListView::itemActivated);
        connect(this, &FeedListView::clicked, this, &FeedListView::itemActivated);
        connect(this->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FeedListView::selectionChanged);
    }


    FeedListView::~FeedListView()
    {
    }

    void FeedListView::itemActivated(const QModelIndex& idx)
    {
        feedActivated(feeds->feedForIndex(idx));
    }

    void FeedListView::selectionChanged(const QItemSelection& sel, const QItemSelection& desel)
    {
        Q_UNUSED(desel);
        Q_UNUSED(sel);
        enableRemove(selectionModel()->selectedRows().count() > 0);
    }

    QModelIndexList FeedListView::selectedFeeds()
    {
        return selectionModel()->selectedRows();
    }
}
