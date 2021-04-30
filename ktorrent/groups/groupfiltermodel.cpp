/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "groupfiltermodel.h"
#include "view/viewmodel.h"
#include <groups/group.h>

namespace kt
{
GroupFilterModel::GroupFilterModel(ViewModel *view_model, QObject *parent)
    : QSortFilterProxyModel(parent)
    , group(nullptr)
    , view_model(view_model)
{
    setSourceModel(view_model);
}

GroupFilterModel::~GroupFilterModel()
{
}

void GroupFilterModel::setGroup(Group *g)
{
    group = g;
    invalidateFilter();
}

void GroupFilterModel::refilter()
{
    invalidateFilter();
}

bool GroupFilterModel::filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_column);
    Q_UNUSED(source_parent);
    return true;
}

bool GroupFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    Q_UNUSED(source_parent);
    if (!group)
        return true;
    else
        return group->isMember(view_model->torrentFromRow(source_row));
}

}
