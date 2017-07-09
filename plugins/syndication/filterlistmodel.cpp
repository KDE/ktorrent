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

#include <QIcon>
#include "filterlistmodel.h"
#include "filter.h"

namespace kt
{

    FilterListModel::FilterListModel(QObject* parent)
        : QAbstractListModel(parent)
    {
    }


    FilterListModel::~FilterListModel()
    {
    }

    void FilterListModel::addFilter(Filter* f)
    {
        filters.append(f);
        insertRow(filters.count() - 1);
    }

    void FilterListModel::removeFilter(Filter* f)
    {
        int idx = filters.indexOf(f);
        beginResetModel();
        filters.removeAll(f);
        if (idx >= 0)
            removeRow(idx);
        endResetModel();
    }

    Filter* FilterListModel::filterForIndex(const QModelIndex& idx)
    {
        if (!idx.isValid())
            return 0;

        return filters.at(idx.row());
    }

    Filter* FilterListModel::filterByName(const QString& name)
    {
        foreach (Filter* f, filters)
            if (f->filterName() == name)
                return f;

        return 0;
    }

    Filter* FilterListModel::filterByID(const QString& id)
    {
        foreach (Filter* f, filters)
            if (f->filterID() == id)
                return f;

        return 0;
    }

    Filter* FilterListModel::filterByRow(int row)
    {
        if (row < 0 || row >= filters.count())
            return 0;
        else
            return filters.at(row);
    }

    int FilterListModel::rowCount(const QModelIndex& parent) const
    {
        if (parent.isValid())
            return 0;
        else
            return filters.count();
    }

    QVariant FilterListModel::data(const QModelIndex& index, int role) const
    {
        if (!index.isValid() || index.row() < 0 || index.row() >= filters.count())
            return QVariant();

        Filter* f = filters.at(index.row());
        if (!f)
            return QVariant();

        switch (role)
        {
        case Qt::DisplayRole:
            return f->filterName();
        case Qt::DecorationRole:
            return QIcon::fromTheme(QStringLiteral("view-filter"));
        }

        return QVariant();
    }

    bool FilterListModel::removeRows(int row, int count, const QModelIndex& parent)
    {
        Q_UNUSED(parent);
        beginRemoveRows(QModelIndex(), row, row + count - 1);
        endRemoveRows();
        return true;
    }

    bool FilterListModel::insertRows(int row, int count, const QModelIndex& parent)
    {
        Q_UNUSED(parent);
        beginInsertRows(QModelIndex(), row, row + count - 1);
        endInsertRows();
        return true;
    }

    void FilterListModel::clear()
    {
        beginResetModel();
        filters.clear();
        endResetModel();
    }
}
