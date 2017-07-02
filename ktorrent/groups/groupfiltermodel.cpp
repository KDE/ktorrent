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

#include <groups/group.h>
#include "groupfiltermodel.h"
#include "view/viewmodel.h"

namespace kt
{

    GroupFilterModel::GroupFilterModel(ViewModel* view_model, QObject* parent)
        : QSortFilterProxyModel(parent), group(nullptr), view_model(view_model)
    {
        setSourceModel(view_model);
    }


    GroupFilterModel::~GroupFilterModel()
    {
    }

    void GroupFilterModel::setGroup(Group* g)
    {
        group = g;
        invalidateFilter();
    }

    void GroupFilterModel::refilter()
    {
        invalidateFilter();
    }

    bool GroupFilterModel::filterAcceptsColumn(int source_column, const QModelIndex& source_parent) const
    {
        Q_UNUSED(source_column);
        Q_UNUSED(source_parent);
        return true;
    }

    bool GroupFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
    {
        Q_UNUSED(source_parent);
        if (!group)
            return true;
        else
            return group->isMember(view_model->torrentFromRow(source_row));
    }

}
