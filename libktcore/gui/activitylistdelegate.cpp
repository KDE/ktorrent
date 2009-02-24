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
#include <QApplication>
#include <QPainter>
#include "activitylistdelegate.h"



namespace kt
{
	ActivityListDelegate::ActivityListDelegate(int icon_size,QObject* parent) 
		: QStyledItemDelegate(parent),icon_size(icon_size),show_icons(true)
	{
	}
	
	ActivityListDelegate::~ActivityListDelegate()
	{
	}

	void ActivityListDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const 
	{
		if (!index.isValid())
			return;
		
		QStyleOptionViewItemV4 optionCopy(*static_cast<const QStyleOptionViewItemV4*>(&option));
		optionCopy.decorationPosition = QStyleOptionViewItem::Top;
		optionCopy.decorationSize = QSize(icon_size,icon_size);
		optionCopy.textElideMode = Qt::ElideNone;
		QStyledItemDelegate::paint(painter,optionCopy,index);
	}

	QSize ActivityListDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const 
	{
		if (!index.isValid())
			return QStyledItemDelegate::sizeHint(option,index);
		
		QStyleOptionViewItemV4 optionCopy(*static_cast<const QStyleOptionViewItemV4*>(&option));
		optionCopy.decorationPosition = QStyleOptionViewItem::Top;
		optionCopy.decorationSize = show_icons ? QSize(icon_size,icon_size) : QSize();
		optionCopy.textElideMode = Qt::ElideNone;
		return QStyledItemDelegate::sizeHint(optionCopy,index);
	}

}