/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KT_TREEFILTERMODEL_H
#define KT_TREEFILTERMODEL_H

#include <QSortFilterProxyModel>
#include <ktcore_export.h>

namespace kt
{
/**
    SortFilterProxyModel for trees, which doesn't
*/
class KTCORE_EXPORT TreeFilterModel : public QSortFilterProxyModel
{
public:
    TreeFilterModel(QObject *parent = nullptr);
    ~TreeFilterModel() override;

protected:
    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

}

#endif // KT_TREEFILTERMODEL_H
