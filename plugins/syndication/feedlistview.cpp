/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "feedlistview.h"
#include "feedlist.h"
#include "feedlistdelegate.h"

namespace kt
{
FeedListView::FeedListView(FeedList *feeds, QWidget *parent)
    : QListView(parent)
    , feeds(feeds)
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

void FeedListView::itemActivated(const QModelIndex &idx)
{
    feedActivated(feeds->feedForIndex(idx));
}

void FeedListView::selectionChanged(const QItemSelection &sel, const QItemSelection &desel)
{
    Q_UNUSED(desel)
    Q_UNUSED(sel)
    enableRemove(selectionModel()->selectedRows().count() > 0);
}

QModelIndexList FeedListView::selectedFeeds()
{
    return selectionModel()->selectedRows();
}
}
