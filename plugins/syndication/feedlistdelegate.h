/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTFEEDLISTDELEGATE_H
#define KTFEEDLISTDELEGATE_H

#include <QStyledItemDelegate>

namespace kt
{
/**
    @author
*/
class FeedListDelegate : public QStyledItemDelegate
{
public:
    FeedListDelegate(QObject *parent);
    ~FeedListDelegate();

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

}

#endif
