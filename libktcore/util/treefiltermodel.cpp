/***************************************************************************
 *   Copyright (C) 2009 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
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

#include "treefiltermodel.h"

namespace kt
{

    TreeFilterModel::TreeFilterModel(QObject* parent): QSortFilterProxyModel(parent)
    {
        setFilterCaseSensitivity(Qt::CaseInsensitive);
    }


    TreeFilterModel::~TreeFilterModel()
    {
    }

    bool TreeFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
    {
        QModelIndex idx = sourceModel()->index(source_row, 0, source_parent);
        if (!idx.isValid())
            return false;

        // if we are in a leaf return filterAcceptsRow
        if (!idx.child(0, 0).isValid())
            return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);

        // now walk over each child recursively and check if one matches, if so we need to accept this
        int child = 0;
        while (idx.child(child, 0).isValid())
        {
            if (filterAcceptsRow(child, idx))
                return true;
            child++;
        }

        return false;
    }

}
