/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "treefiltermodel.h"

namespace kt
{
TreeFilterModel::TreeFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

TreeFilterModel::~TreeFilterModel()
{
}

bool TreeFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
    if (!idx.isValid())
        return false;

    // if we are in a leaf return filterAcceptsRow
    if (!sourceModel()->hasIndex(0, 0, idx))
        return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);

    // now walk over each child recursively and check if one matches, if so we need to accept this
    int child = 0;
    while (sourceModel()->hasIndex(child, 0, idx)) {
        if (filterAcceptsRow(child, idx))
            return true;
        child++;
    }

    return false;
}

}
