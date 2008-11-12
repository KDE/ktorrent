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
#include <QPainter>
#include <QTextDocument>
#include "feedlistdelegate.h"

namespace kt 
{

	FeedListDelegate::FeedListDelegate(QObject* parent) : QItemDelegate(parent)
	{
	}
	
	
	FeedListDelegate::~FeedListDelegate()
	{
	}
	
	QSize FeedListDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
	{
		QString text = index.data(Qt::UserRole).toString();
		QTextDocument doc;
		doc.setHtml(text);
		QSizeF s = doc.size();
		int h = (int)s.height();
		if (option.decorationSize.height() > h)
			h = option.decorationSize.height();
		return QSize((int)s.width() + option.decorationSize.width(),h);
	}
	
	void FeedListDelegate::paint(QPainter * painter,const QStyleOptionViewItem & option,const QModelIndex & index) const
	{
		QString text = index.data(Qt::UserRole).toString();
		
		// If selected fill the rect with the highlight color
		if (option.state & QStyle::State_Selected)
			painter->fillRect(option.rect, option.palette.highlight());
		
		painter->setBrush(option.palette.foreground());

		// Draw the text
		painter->save();
		QTextDocument doc;
		doc.setHtml(text);
		doc.setPageSize(option.rect.size());
		painter->translate(option.rect.x() + option.decorationSize.width(), option.rect.y());
		doc.drawContents(painter);
		painter->restore();
		
		// Draw the decoration
		QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
		QPixmap pixmap = icon.pixmap(option.decorationSize);
		QRect source_rect(0,0,option.decorationSize.width(),option.decorationSize.height());
		QRect dest_rect = source_rect;
		int y = (option.rect.height() - source_rect.height()) / 2;
		dest_rect.setY(y);
		dest_rect.setHeight(dest_rect.height() + y);
		painter->save();
		painter->drawPixmap(dest_rect,pixmap,source_rect);
		painter->restore();
	}
}
