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

#include <QAbstractItemModel>
#include "itemselectionmodel.h"

namespace kt
{
    ItemSelectionModel::ItemSelectionModel(QAbstractItemModel* model, QObject* parent)
        : QItemSelectionModel(model, parent)
    {}

    ItemSelectionModel::~ItemSelectionModel()
    {}

    void ItemSelectionModel::select(const QModelIndex& index, QItemSelectionModel::SelectionFlags command)
    {
        QItemSelection sel(index, index);
        select(sel, command);
    }

    void ItemSelectionModel::select(const QItemSelection& sel, QItemSelectionModel::SelectionFlags command)
    {
        if (command == NoUpdate)
            return;

        if (command & QItemSelectionModel::Clear)
            selection.clear();

        for (const QItemSelectionRange& r : sel)
            doRange(r, command);

        QItemSelectionModel::select(sel, command);
    }

    void ItemSelectionModel::doRange(const QItemSelectionRange r, QItemSelectionModel::SelectionFlags command)
    {
        for (int i = r.topLeft().row(); i <= r.bottomRight().row(); i++)
        {
            void* item = model()->index(i, 0).internalPointer();
            if (!item)
                continue;

            if (command & QItemSelectionModel::Select)
            {
                selection.insert(item);
            }
            else if (command & QItemSelectionModel::Deselect)
            {
                selection.remove(item);
            }
            else if (command & QItemSelectionModel::Toggle)
            {
                if (selection.contains(item))
                    selection.remove(item);
                else
                    selection.insert(item);
            }
        }
    }

    void ItemSelectionModel::reset()
    {
        selection.clear();
        QItemSelectionModel::reset();
    }

    void ItemSelectionModel::clear()
    {
        selection.clear();
        QItemSelectionModel::clear();
    }

    void ItemSelectionModel::sorted()
    {
        QItemSelection ns;
        int rows = model()->rowCount(QModelIndex());
        int cols = model()->columnCount(QModelIndex());
        for (int i = 0; i < rows; i++)
        {
            QModelIndex idx = model()->index(i, 0, QModelIndex());
            void* item = idx.internalPointer();
            if (item && selection.contains(item))
            {
                ns.select(idx, model()->index(i, cols - 1, QModelIndex()));
            }
        }

        select(ns, QItemSelectionModel::ClearAndSelect);
    }
}
