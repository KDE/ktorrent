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

#define ITEM_MARGIN_LEFT 5
#define ITEM_MARGIN_TOP 5
#define ITEM_MARGIN_RIGHT 5
#define ITEM_MARGIN_BOTTOM 5
#define ITEM_PADDING 5

namespace kt
{
	ActivityListDelegate::ActivityListDelegate(QObject* parent) : QStyledItemDelegate(parent)
	{
	}
	
	ActivityListDelegate::~ActivityListDelegate()
	{
	}

	void ActivityListDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const 
	{
		QBrush back_brush;
		QColor fore_color;
		bool disabled = false;
		bool hover = false;
		if (!(option.state & QStyle::State_Enabled))
		{
			back_brush = option.palette.brush(QPalette::Disabled, QPalette::Base);
			fore_color = option.palette.color(QPalette::Disabled, QPalette::Text);
			disabled = true;
		}
		else if (option.state & (QStyle::State_HasFocus | QStyle::State_Selected))
		{
			back_brush = option.palette.brush(QPalette::Highlight);
			fore_color = option.palette.color(QPalette::HighlightedText);
		}
		else if (option.state & QStyle::State_MouseOver)
		{
			back_brush = option.palette.color(QPalette::Highlight).light( 115 );
			fore_color = option.palette.color(QPalette::HighlightedText);
			hover = true;
		}
		else
		{
			back_brush = option.palette.brush(QPalette::Base);
			fore_color = option.palette.color(QPalette::Text);
		}
		
		QStyle* style = QApplication::style();
		QStyleOptionViewItemV4 opt(option);
		// KStyle provides an "hover highlight" effect for free;
		// but we want that for non-KStyle-based styles too
		if (!style->inherits("KStyle") && hover)
		{
			Qt::BrushStyle bs = opt.backgroundBrush.style();
			if (bs > Qt::NoBrush && bs < Qt::TexturePattern)
				opt.backgroundBrush = opt.backgroundBrush.color().light(115);
			else
				opt.backgroundBrush = back_brush;
		}
		painter->save();
		style->drawPrimitive(QStyle::PE_PanelItemViewItem,&opt,painter,0);
		painter->restore();
		QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();
		if (!icon.isNull())
		{
			QPoint iconpos((option.rect.width() - option.decorationSize.width()) / 2,ITEM_MARGIN_TOP);
			iconpos += option.rect.topLeft();
			QIcon::Mode iconmode = disabled ? QIcon::Disabled : QIcon::Normal;
			painter->drawPixmap(iconpos, icon.pixmap( option.decorationSize, iconmode));
		}
		
		
		QString text = index.data(Qt::DisplayRole).toString();
		QStringList splitted = text.split("\n");
		int y = ITEM_MARGIN_TOP + option.decorationSize.height() + ITEM_PADDING;
		foreach (const QString & s,splitted)
		{
			QFontMetrics fm(option.font);
			QRect font_bounds = fm.boundingRect(s);
			int x = ITEM_MARGIN_LEFT + (option.rect.width() - ITEM_MARGIN_LEFT - ITEM_MARGIN_RIGHT - font_bounds.width()) / 2;
			QPoint textPos(x,y);
			y += fm.height();
			font_bounds.translate(-font_bounds.topLeft());
			font_bounds.translate(textPos);
			font_bounds.translate(option.rect.topLeft());
			painter->setPen(fore_color);
			painter->drawText(font_bounds, Qt::AlignCenter, s);
		}
	}

	QSize ActivityListDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const 
	{
		QSize base_size(option.decorationSize.width(), option.decorationSize.height());
		QString text = index.data(Qt::DisplayRole).toString();
		QStringList splitted = text.split("\n");

		int w = 0;
		int h = 0;
		foreach (const QString & s,splitted)
		{
			QFontMetrics fm(option.font);
			h += fm.height();
			int nw = fm.width(s);
			if (nw > w)
				w = nw;
		}
		base_size.setWidth(qMax(w,base_size.width()));
		base_size.setHeight(base_size.height() + h + ITEM_PADDING);
		return base_size + QSize(ITEM_MARGIN_LEFT + ITEM_MARGIN_RIGHT, ITEM_MARGIN_TOP + ITEM_MARGIN_BOTTOM);
	}

}