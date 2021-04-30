/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTGROUPFILTERMODEL_H
#define KTGROUPFILTERMODEL_H

#include <QSortFilterProxyModel>

namespace kt
{
class Group;
class ViewModel;

/**
    Model to filter out torrents based upon group membership
*/
class GroupFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    GroupFilterModel(ViewModel *view_model, QObject *parent);
    ~GroupFilterModel() override;

    bool filterAcceptsColumn(int source_column, const QModelIndex &source_parent) const override;
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

    /**
     * Set the group to filter
     * @param g The Group
     * */
    void setGroup(Group *g);

    /**
     * Filter again.
     */
    void refilter();

private:
    Group *group;
    ViewModel *view_model;
};

}

#endif
