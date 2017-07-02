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

#include "schedulegraphicsitem.h"

#include <QPen>
#include <QBrush>
#include <QRectF>
#include <QCursor>
#include <QFontMetricsF>
#include <QGraphicsTextItem>
#include <QGraphicsSceneMouseEvent>

#include <KCursor>
#include <KLocalizedString>

#include <util/log.h>
#include <util/functions.h>
#include "schedule.h"
#include "weekscene.h"
#include "bwschedulerpluginsettings.h"

using namespace bt;

namespace kt
{
    const Uint32 Top = 1;
    const Uint32 Bottom = 2;
    const Uint32 Left = 4;
    const Uint32 Right = 8;

    const Uint32 TopRight = Top | Right;
    const Uint32 TopLeft = Top | Left;
    const Uint32 BottomRight = Bottom | Right;
    const Uint32 BottomLeft = Bottom | Left;

    ScheduleGraphicsItem::ScheduleGraphicsItem(ScheduleItem* item, const QRectF& r, const QRectF& constraints, WeekScene* ws)
        : QGraphicsRectItem(r)
        , item(item)
        , constraints(constraints)
        , ws(ws)
        , text_item(nullptr)
        , resize_edge(0)
        , ready_to_resize(false)
        , resizing(false)
    {
        setAcceptHoverEvents(true);
        setPen(QPen(Qt::black));
        setZValue(3);
        setHandlesChildEvents(true);

        setBrush(QBrush(item->suspended?
                                SchedulerPluginSettings::suspendedColor()
                               :SchedulerPluginSettings::itemColor()));
        setFlag(QGraphicsItem::ItemIsSelectable, true);
        setFlag(QGraphicsItem::ItemIsMovable, true);
    }


    ScheduleGraphicsItem::~ScheduleGraphicsItem()
    {
    }

    void ScheduleGraphicsItem::update(const QRectF& r)
    {
        setRect(r);
        setPos(QPointF(0, 0));
        QString text;
        if (item->suspended)
        {
            setBrush(QBrush(SchedulerPluginSettings::suspendedColor()));
            text = i18n("Suspended");
        }
        else
        {
            setBrush(QBrush(SchedulerPluginSettings::itemColor()));
            QString ds = item->download_limit == 0 ? i18n("Unlimited") : BytesPerSecToString(item->download_limit * 1024);
            QString us = item->upload_limit == 0 ? i18n("Unlimited") : BytesPerSecToString(item->upload_limit * 1024);
            text = i18n("%1 Down\n%2 Up", ds, us);
        }

        if (text_item == nullptr)
            text_item = scene()->addText(text);
        else
            text_item->setPlainText(text);

        QFontMetricsF fm(text_item->font());
        text_item->setPos(QPointF(r.x(), r.y()));
        text_item->setZValue(4);
        text_item->setTextWidth(r.width());
        text_item->setParentItem(this);
        setToolTip(text);

        if (text_item->boundingRect().height() > r.height())
        {
            // Text is to big for rect
            delete text_item;
            text_item = nullptr;
        }
    }

    QVariant ScheduleGraphicsItem::itemChange(GraphicsItemChange change, const QVariant& value)
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

    QRectF ScheduleGraphicsItem::resize(QPointF scene_pos)
    {
        qreal x = scene_pos.x();
        qreal y = scene_pos.y();

        QRectF cur = rect();
        if (resize_edge & Top)
        {
            if (y >= cur.y() + cur.height()) // rect becomes flipped
            {
                qreal yn = cur.y() + cur.height();
                if (yn < constraints.y())
                    yn = constraints.y();

                qreal h = y - yn;
                cur.setY(yn);
                cur.setHeight(h);
                resize_edge |= kt::Bottom;
                resize_edge &= ~kt::Top;
            }
            else
            {
                qreal yn = y < constraints.y() ? constraints.y() : y;
                qreal h = cur.height() + (cur.y() - yn);
                cur.setY(yn);
                cur.setHeight(h);
            }
        }
        else if (resize_edge & Bottom)
        {
            if (y < cur.y()) // rect becomes flipped
            {
                qreal yn = y;
                if (yn < constraints.y())
                    yn = constraints.y();

                qreal h = cur.y() - yn;
                cur.setY(yn);
                cur.setHeight(h);
                resize_edge |= kt::Top;
                resize_edge &= ~kt::Bottom;
            }
            else
            {
                cur.setHeight(y - cur.y());
                if (cur.y() + cur.height() >= constraints.y() + constraints.height())
                    cur.setHeight(constraints.y() + constraints.height() - cur.y());
            }
        }

        if (resize_edge & Left)
        {
            if (x >= cur.x() + cur.width()) // rect becomes flipped
            {
                qreal xn = cur.x() + cur.x();
                if (xn < constraints.x())
                    xn = constraints.x();

                qreal w = x - xn;
                cur.setX(xn);
                cur.setWidth(w);
                resize_edge |= kt::Right;
                resize_edge &= ~kt::Left;
            }
            else
            {
                qreal xn = x < constraints.x() ? constraints.x() : x;
                qreal w = cur.width() + (cur.x() - xn);
                cur.setX(xn);
                cur.setWidth(w);
            }
        }
        else if (resize_edge & Right)
        {
            if (x < cur.x()) // rect becomes flipped
            {
                qreal xn = x;
                if (xn < constraints.x())
                    xn = constraints.x();

                qreal w = cur.x() - xn;
                cur.setX(xn);
                cur.setWidth(w);
                resize_edge |= kt::Left;
                resize_edge &= ~kt::Right;
            }
            else
            {
                cur.setWidth(x - cur.x());
                if (cur.x() + cur.width() >= constraints.x() + constraints.width())
                    cur.setWidth(constraints.x() + constraints.width() - cur.x());
            }
        }

        return cur;
    }


    void ScheduleGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
    {
        if (!resizing)
        {
            QGraphicsItem::mouseMoveEvent(event);
            ws->setShowGuidanceLines(true);
            QPointF sp = pos() + rect().topLeft();
            ws->updateGuidanceLines(sp.y(), sp.y() + rect().height());

            setCursor(ws->validMove(item, sp)?Qt::DragMoveCursor:Qt::ForbiddenCursor);
        }
        else
        {
            QRectF cur = resize(event->scenePos());
            setRect(cur);
            if (text_item)
                text_item->setPos(cur.x(), cur.y());

            ws->updateGuidanceLines(cur.y(), cur.y() + cur.height());
        }
    }

    void ScheduleGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
    {
        if (!ready_to_resize || !(event->button() & Qt::LeftButton))
        {
            QGraphicsRectItem::mousePressEvent(event);
            // keep track of original position before the item is dragged
            original_pos = pos();
        }
        else
        {
            resizing = true;
            ws->setShowGuidanceLines(true);
            ws->updateGuidanceLines(rect().y(), rect().y() + rect().height());
        }

        setZValue(4);
    }

    void ScheduleGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
    {
        if (resizing)
        {
            resizing = false;
            ws->setShowGuidanceLines(false);
            ws->itemResized(item, rect());
        }
        else
        {
            QGraphicsRectItem::mouseReleaseEvent(event);

            if (event->button() & Qt::LeftButton)
            {
                if (original_pos != pos())
                {
                    QPointF sp = pos() + rect().topLeft();
                    ws->itemMoved(item, sp);
                }
            }
            ws->setShowGuidanceLines(false);
        }

        setZValue(3);
        setCursor(Qt::ArrowCursor);
    }

    void ScheduleGraphicsItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
    {
        ready_to_resize = true;
        resize_edge = nearEdge(event->scenePos());
        updateCursor();
    }

    void ScheduleGraphicsItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
    {
        Q_UNUSED(event);
        setCursor(Qt::ArrowCursor);
        ready_to_resize = false;
    }

    void ScheduleGraphicsItem::hoverMoveEvent(QGraphicsSceneHoverEvent* event)
    {
        resize_edge = nearEdge(event->scenePos());
        ready_to_resize = resize_edge != 0;
        updateCursor();
    }

    void ScheduleGraphicsItem::updateCursor()
    {
        Qt::CursorShape shape = Qt::ArrowCursor;
        if (resize_edge != 0)
        {
            if (resize_edge == kt::TopRight || resize_edge == kt::BottomLeft)
                shape = Qt::SizeBDiagCursor;
            else if (resize_edge == kt::BottomRight || resize_edge == kt::TopLeft)
                shape = Qt::SizeFDiagCursor;
            else if (resize_edge == kt::Top || resize_edge == kt::Bottom)
                shape = Qt::SizeVerCursor;
            else
                shape = Qt::SizeHorCursor;
        }
        setCursor(shape);
    }


    Uint32 ScheduleGraphicsItem::nearEdge(QPointF p)
    {
        qreal y = rect().y();
        qreal ye = y + rect().height();
        qreal x = rect().x();
        qreal xe = x + rect().width();
        Uint32 ret = 0;
        if (qAbs(p.y() - y) < 4)
            ret |= Top;
        else if (qAbs(p.y() - ye) < 4)
            ret |= Bottom;

        if (qAbs(p.x() - x) < 4)
            ret |= Left;
        else if (qAbs(p.x() - xe) < 4)
            ret |= Right;

        return ret;
    }


}
