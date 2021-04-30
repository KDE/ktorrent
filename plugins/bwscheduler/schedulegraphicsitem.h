/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTSCHEDULEGRAPHICSITEM_H
#define KTSCHEDULEGRAPHICSITEM_H

#include "schedule.h"
#include <QGraphicsRectItem>

namespace kt
{
class WeekScene;

/**
    QGraphicsItem to display a ScheduleItem
*/
class ScheduleGraphicsItem : public QGraphicsRectItem
{
public:
    ScheduleGraphicsItem(ScheduleItem *item, const QRectF &r, const QRectF &constraints, WeekScene *ws);
    ~ScheduleGraphicsItem() override;

    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    /**
     * Update the item.
     * @param r The new rect
     */
    void update(const QRectF &r);

private:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    bt::Uint32 nearEdge(QPointF p);
    QRectF resize(QPointF scene_pos);
    void updateCursor();

private:
    ScheduleItem *item;
    QRectF constraints;
    WeekScene *ws;
    QGraphicsTextItem *text_item;
    QPointF original_pos;
    bt::Uint32 resize_edge;
    bool ready_to_resize;
    bool resizing;
};

}

#endif
