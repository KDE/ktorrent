/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTFILTERLISTMODEL_H
#define KTFILTERLISTMODEL_H

#include <QAbstractListModel>

namespace kt
{
class Filter;

/**
    Model to show a list of filters in a view.
*/
class FilterListModel : public QAbstractListModel
{
public:
    FilterListModel(QObject *parent);
    ~FilterListModel();

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool removeRows(int row, int count, const QModelIndex &parent) override;
    bool insertRows(int row, int count, const QModelIndex &parent) override;

    void addFilter(Filter *f);
    void removeFilter(Filter *f);
    Filter *filterForIndex(const QModelIndex &idx);
    Filter *filterByName(const QString &name);
    Filter *filterByRow(int row);
    Filter *filterByID(const QString &id);
    void clear();

protected:
    QList<Filter *> filters;
};

}

#endif
