/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "filterlistview.h"
#include "filterlist.h"

namespace kt
{
FilterListView::FilterListView(FilterList *filters, QWidget *parent)
    : QListView(parent)
    , filters(filters)
{
    setContextMenuPolicy(Qt::CustomContextMenu);
    setModel(filters);
    setAlternatingRowColors(true);
    connect(this, &FilterListView::doubleClicked, this, &FilterListView::itemActivated);
    connect(this->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FilterListView::selectionChanged);
}

FilterListView::~FilterListView()
{
}

void FilterListView::itemActivated(const QModelIndex &idx)
{
    filterActivated(filters->filterForIndex(idx));
}

void FilterListView::selectionChanged(const QItemSelection &sel, const QItemSelection &desel)
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
