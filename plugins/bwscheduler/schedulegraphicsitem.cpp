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
#include <QPen>
#include <QBrush>
#include <QRectF>
#include <QFontMetricsF>
#include <QGraphicsTextItem>
#include <QGraphicsSceneMouseEvent>
#include <klocale.h>
#include <util/log.h>
#include <util/functions.h>
#include "schedulegraphicsitem.h"
#include "schedule.h"
#include "weekscene.h"

using namespace bt;

namespace kt
{

	ScheduleGraphicsItem::ScheduleGraphicsItem(ScheduleItem* item,const QRectF & r,const QRectF & constraints,WeekScene* ws)
	: QGraphicsRectItem(r),item(item),constraints(constraints),ws(ws)
	{
		setPen(QPen(Qt::black));
		setZValue(3);
		
		if (item->paused)
		{
			setBrush(QBrush(QColor(255,0,0,255)));
		}
		else
		{
			setBrush(QBrush(QColor(0,255,0,255)));
		}
		setFlag(QGraphicsItem::ItemIsSelectable,true);
		setFlag(QGraphicsItem::ItemIsMovable,true);
		text_item = 0;
	}


	ScheduleGraphicsItem::~ScheduleGraphicsItem()
	{
	}
	
	void ScheduleGraphicsItem::update(const QRectF & r)
	{
		setRect(r);
		setPos(QPointF(0,0));
		QString text;
		if (item->paused)
		{
			setBrush(QBrush(QColor(255,0,0,255)));
			text = i18n("Paused");
		}
		else
		{
			setBrush(QBrush(QColor(0,255,0,255)));
			text = i18n("%1 Down\n%2 Up",
						KBytesPerSecToString(item->download_limit),
											 KBytesPerSecToString(item->upload_limit));
		}
		
		if (text_item == 0)
			text_item = scene()->addText(text);
		else
			text_item->setPlainText(text);
		
		QFontMetricsF fm(text_item->font());
		text_item->setPos(QPointF(r.x(),r.y()));
		text_item->setZValue(4);
		text_item->setTextWidth(r.width());
		text_item->setParentItem(this);
		setToolTip(text);
		
		if (text_item->boundingRect().height() > r.height())
		{
			// Text is to big for rect
			delete text_item;
			text_item = 0;
		}
	}

	QVariant ScheduleGraphicsItem::itemChange(GraphicsItemChange change, const QVariant &value)
	{
		if (change == ItemPositionChange && scene()) 
		{
			QPointF new_pos = value.toPointF();
			if (!constraints.contains(new_pos))
			{
				qreal x = constraints.x() - boundingRect().x();
				if (new_pos.x() < x)
					new_pos.setX(x);
				else if (new_pos.x() + rect().width() > x + constraints.width())
					new_pos.setX(x + constraints.width() - rect().width());
				
				qreal y = constraints.y() - boundingRect().y();
				if (new_pos.y() < y)
					new_pos.setY(y);
				else if (new_pos.y() + rect().height() > y + constraints.height())
					new_pos.setY(y + constraints.height() - rect().height());
				
				return new_pos;
			}
		}

		return QGraphicsItem::itemChange(change, value);
	}
	
	void ScheduleGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
	{
		QGraphicsItem::mouseMoveEvent(event);
	}
	
	void ScheduleGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
	{
		QGraphicsRectItem::mousePressEvent(event);
		// keep track of original position before the item is dragged
		original_pos = pos();
	}
	
	void ScheduleGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
	{
		QGraphicsRectItem::mouseReleaseEvent(event);
		
		if (event->button() & Qt::LeftButton)
		{
			if (original_pos != pos())
			{
				QPointF sp = pos() + rect().topLeft();
				ws->itemMoved(item,sp);
			}
		}
	}

}
