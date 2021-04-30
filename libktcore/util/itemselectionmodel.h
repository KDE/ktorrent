/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ITEMSELECTIONMODEL_H
#define ITEMSELECTIONMODEL_H

#include <QItemSelectionModel>
#include <QSet>

#include <ktcore_export.h>

namespace kt
{
/**
 * Selection model which works on internal pointers instead of indexes.
 */
class KTCORE_EXPORT ItemSelectionModel : public QItemSelectionModel
{
    Q_OBJECT
public:
    ItemSelectionModel(QAbstractItemModel *model, QObject *parent);
    ~ItemSelectionModel() override;

    void select(const QModelIndex &index, QItemSelectionModel::SelectionFlags command) override;
    void select(const QItemSelection &sel, QItemSelectionModel::SelectionFlags command) override;
    void clear() override;
    void reset() override;

public Q_SLOTS:
    /**
     * Updates the selection after a sort.
     */
    void sorted();

private:
    void doRange(const QItemSelectionRange r, QItemSelectionModel::SelectionFlags command);

private:
    QSet<void *> selection;
};
}

#endif // ITEMSELECTIONMODEL_H
