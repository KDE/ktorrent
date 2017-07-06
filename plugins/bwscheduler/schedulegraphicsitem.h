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
        ScheduleGraphicsItem(ScheduleItem* item, const QRectF& r, const QRectF& constraints, WeekScene* ws);
        ~ScheduleGraphicsItem();

        QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

        /**
         * Update the item.
         * @param r The new rect
         */
        void update(const QRectF& r);

    private:
        void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
        void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
        void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
        void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
        void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
        void hoverMoveEvent(QGraphicsSceneHoverEvent* event) override;
        bt::Uint32 nearEdge(QPointF p);
        QRectF resize(QPointF scene_pos);
        void updateCursor();

    private:
        ScheduleItem* item;
        QRectF constraints;
        WeekScene* ws;
        QGraphicsTextItem* text_item;
        QPointF original_pos;
        bt::Uint32 resize_edge;
        bool ready_to_resize;
        bool resizing;
    };

}

#endif
