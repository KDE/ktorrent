/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTLOGFLAGSDELEGATE_H
#define KTLOGFLAGSDELEGATE_H

#include <QItemDelegate>

namespace kt
{
/**
    @author
*/
class LogFlagsDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    LogFlagsDelegate(QObject *parent);
    ~LogFlagsDelegate() override;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

}

#endif
