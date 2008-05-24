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
#ifndef KTSCHEDULEGRAPHICSITEM_H
#define KTSCHEDULEGRAPHICSITEM_H

#include <QGraphicsRectItem>
#include "schedule.h"

namespace kt
{
	class WeekScene;

	/**
		QGraphicsItem to display a ScheduleItem
	*/
	class ScheduleGraphicsItem : public QGraphicsRectItem
	{
	public:
		ScheduleGraphicsItem(ScheduleItem* item,const QRectF & r,const QRectF & constraints,WeekScene* ws);
		virtual ~ScheduleGraphicsItem();

		virtual QVariant itemChange(GraphicsItemChange change, const QVariant & value);
		
		/**
		 * Update the item.
		 * @param r The new rect
		 * @param cst The new constraints
		 */
		void update(const QRectF & r,const QRectF & cst);
		
	private:
		virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
		virtual void mousePressEvent(QGraphicsSceneMouseEvent* event);
		virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);

	private:
		ScheduleItem* item;
		QRectF constraints;
		WeekScene* ws;
		QGraphicsTextItem* text_item;
		QPointF original_pos;
	};

}

#endif
