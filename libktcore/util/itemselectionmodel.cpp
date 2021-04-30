/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "itemselectionmodel.h"
#include <QAbstractItemModel>

namespace kt
{
ItemSelectionModel::ItemSelectionModel(QAbstractItemModel *model, QObject *parent)
    : QItemSelectionModel(model, parent)
{
}

ItemSelectionModel::~ItemSelectionModel()
{
}

void ItemSelectionModel::select(const QModelIndex &index, QItemSelectionModel::SelectionFlags command)
{
    QItemSelection sel(index, index);
    select(sel, command);
}

void ItemSelectionModel::select(const QItemSelection &sel, QItemSelectionModel::SelectionFlags command)
{
    if (command == NoUpdate)
        return;

    if (command & QItemSelectionModel::Clear)
        selection.clear();

    for (const QItemSelectionRange &r : sel)
        doRange(r, command);

    QItemSelectionModel::select(sel, command);
}

void ItemSelectionModel::doRange(const QItemSelectionRange r, QItemSelectionModel::SelectionFlags command)
{
    for (int i = r.topLeft().row(); i <= r.bottomRight().row(); i++) {
        void *item = model()->index(i, 0).internalPointer();
        if (!item)
            continue;

        if (command & QItemSelectionModel::Select) {
            selection.insert(item);
        } else if (command & QItemSelectionModel::Deselect) {
            selection.remove(item);
        } else if (command & QItemSelectionModel::Toggle) {
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
    for (int i = 0; i < rows; i++) {
        QModelIndex idx = model()->index(i, 0, QModelIndex());
        void *item = idx.internalPointer();
        if (item && selection.contains(item)) {
            ns.select(idx, model()->index(i, cols - 1, QModelIndex()));
        }
    }

    select(ns, QItemSelectionModel::ClearAndSelect);
}
}
