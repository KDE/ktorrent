/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QApplication>
#include <QPainter>
#include <QStyleOptionViewItem>

#include "feedlistdelegate.h"

namespace kt
{
FeedListDelegate::FeedListDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

FeedListDelegate::~FeedListDelegate()
{
}

QSize FeedListDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QVariant value = index.data(Qt::SizeHintRole);
    if (value.isValid())
        return qvariant_cast<QSize>(value);

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    opt.text = displayText(index.data(Qt::UserRole).toString(), opt.locale);

    const QWidget *widget = opt.widget;
    QStyle *style = widget ? widget->style() : QApplication::style();
    return style->sizeFromContents(QStyle::CT_ItemViewItem, &opt, QSize(), widget);
}

void FeedListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    opt.text = displayText(index.data(Qt::UserRole).toString(), opt.locale);

    const QWidget *widget = opt.widget;

    QStyle *style = widget ? widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);
}
}
