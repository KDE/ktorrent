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

#include <QApplication>
#include <QPainter>
#include <QStyleOptionViewItem>

#include "feedlistdelegate.h"

namespace kt
{

    FeedListDelegate::FeedListDelegate(QObject* parent) : QStyledItemDelegate(parent)
    {
    }


    FeedListDelegate::~FeedListDelegate()
    {
    }

    QSize FeedListDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        QVariant value = index.data(Qt::SizeHintRole);
        if (value.isValid())
            return qvariant_cast<QSize>(value);

        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        opt.text = displayText(index.data(Qt::UserRole).toString(), opt.locale);

        const QWidget* widget = opt.widget;
        QStyle* style = widget ? widget->style() : QApplication::style();
        return style->sizeFromContents(QStyle::CT_ItemViewItem, &opt, QSize(), widget);
    }


    void FeedListDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        opt.text = displayText(index.data(Qt::UserRole).toString(), opt.locale);

        const QWidget* widget = opt.widget;

        QStyle* style = widget ? widget->style() : QApplication::style();
        style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);
    }
}
