/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
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

#include "weekscene.h"

#include <cmath>

#include <KFormat>
#include <KLocalizedString>

#include <QFontMetricsF>
#include <QGraphicsLineItem>
#include <QGraphicsRectItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsTextItem>
#include <QPalette>

#include <util/functions.h>
#include <util/log.h>
#include "schedule.h"
#include "schedulegraphicsitem.h"
#include "bwschedulerpluginsettings.h"
#include "guidanceline.h"


using namespace bt;

namespace kt
{

    WeekScene::WeekScene(QObject* parent)
        : QGraphicsScene(parent),
          schedule(nullptr)
    {
        addCalendar();
    }


    WeekScene::~WeekScene()
    {}

    qreal LongestDayWidth(const QFontMetricsF& fm)
    {
        qreal wd = 0;
        for (int i = 1; i <= 7; i++)
        {
            qreal w = fm.width(QLocale::system().dayName(i));
            if (w > wd)
                wd = w;
        }
        return wd;
    }

    void WeekScene::updateStatusText(int up, int down, bool suspended, bool enabled)
    {
        static KFormat format;
        QString msg;
        if (suspended)
            msg = i18n("Current schedule: suspended");
        else if (up > 0 && down > 0)
            msg = i18n("Current schedule: %1/s download, %2/s upload",
                       format.formatByteSize(down * 1024), format.formatByteSize(up * 1024));
        else if (up > 0)
            msg = i18n("Current schedule: unlimited download, %1/s upload", format.formatByteSize(up * 1024));
        else if (down > 0)
            msg = i18n("Current schedule: %1/s download, unlimited upload", format.formatByteSize(down * 1024));
        else
            msg = i18n("Current schedule: unlimited upload and download");

        if (!enabled)
            msg += i18n(" (scheduler disabled)");

        status->setPlainText(msg);
    }

    void WeekScene::addCalendar()
    {
        QGraphicsTextItem* tmp = addText(QStringLiteral("Dinges"));
        QFontMetricsF fm(tmp->font());
        removeItem(tmp);
        delete tmp;


        // first add 7 rectangles for each day of the week
        xoff = fm.width(QStringLiteral("00:00")) + 10;
        yoff = 2 * fm.height() + 10;
        day_width = LongestDayWidth(fm) * 1.5;
        hour_height = fm.height() * 1.5;

        status = addText(i18n("Current schedule:"));
        status->setPos(QPointF(0, 0));
        status->setZValue(2);

        QPen pen(SchedulerPluginSettings::scheduleLineColor());
        QBrush brush(SchedulerPluginSettings::scheduleBackgroundColor());

        for (int i = 0; i < 7; i++)
        {
            QGraphicsRectItem* item = addRect(xoff + day_width * i, yoff, day_width, 24 * hour_height, pen, brush);
            item->setZValue(1);

            QString day = QLocale::system().dayName(i + 1);

            // make sure day is centered in the middle of the column
            qreal dlen = fm.width(day);
            qreal mid = xoff + day_width * (i + 0.5);
            qreal start = mid - dlen * 0.5;

            QGraphicsTextItem* t = addText(day);
            t->setPos(QPointF(start, fm.height() + 5));
            t->setZValue(2);

            rects.append(item);
        }

        // draw hour lines
        for (int i = 0; i <= 24; i++)
        {
            QGraphicsLineItem* item = addLine(0, yoff + i * hour_height, xoff + 7 * day_width, yoff + i * hour_height, pen);
            item->setZValue(2);

            if (i < 24)
            {
                QGraphicsTextItem* t = addText(QStringLiteral("%1:00").arg(i));
                t->setPos(QPointF(0, yoff + i * hour_height));
                t->setZValue(2);
            }
            lines.append(item);
        }

        ;
        gline[0] = new GuidanceLine(xoff, yoff, xoff + 7 * day_width + 10);
        gline[0]->setVisible(false);
        gline[1] = new GuidanceLine(xoff, yoff, xoff + 7 * day_width + 10);
        gline[1]->setVisible(false);
        addItem(gline[0]);
        addItem(gline[1]);

        QRectF r = sceneRect();
        r.setHeight(r.height() + 10);
        setSceneRect(r);
    }

    QGraphicsItem* WeekScene::addScheduleItem(ScheduleItem* item)
    {
        QTime midnight(0, 0, 0, 0);
        qreal x = xoff + (item->start_day - 1) * day_width;
//      qreal min_h = hour_height / 60.0;
        qreal y = timeToY(item->start);
        qreal ye = timeToY(item->end);

        QRectF rect(x, y, day_width * (item->end_day - item->start_day + 1), ye - y);
        QRectF cst(xoff, yoff, 7 * day_width, 24 * hour_height);
        ScheduleGraphicsItem* gi = new ScheduleGraphicsItem(item, rect, cst, this);
        addItem(gi);
        gi->update(rect);
        return gi;
    }

    void WeekScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* ev)
    {
        const QList<QGraphicsItem*> gis = items(ev->scenePos());
        for (QGraphicsItem* gi : gis)
        {
            if (gi->zValue() == 3)
            {
                itemDoubleClicked(gi);
                break;
            }
        }
    }

    void WeekScene::mousePressEvent(QGraphicsSceneMouseEvent* ev)
    {
        if (ev->button() == Qt::RightButton)
        {
            QList<QGraphicsItem*> gis = items(ev->scenePos());
            foreach (QGraphicsItem* gi, gis)
            {
                if (gi->zValue() == 3)
                {
                    clearSelection();
                    gi->setSelected(true);
                    break;
                }
            }
        }
        else
            QGraphicsScene::mousePressEvent(ev);
    }

    qreal WeekScene::timeToY(const QTime& time)
    {
        QTime midnight(0, 0, 0, 0);
        qreal min_h = hour_height / 60.0;
        return (midnight.secsTo(time) / 60.0) * min_h + yoff;
    }

    QTime WeekScene::yToTime(qreal y)
    {
        y = y - yoff; // get rid of offset
        qreal min_h = hour_height / 60.0;
        return QTime(0, 0, 0, 0).addSecs((y / min_h) * 60);
    }

    void WeekScene::itemMoved(ScheduleItem* item, const QPointF& np)
    {
        QTime start = yToTime(np.y());
        int d = item->start.secsTo(item->end); // duration in seconds
        QTime end = start.addSecs(d);

        int start_day = 1 + floor((np.x() + day_width * 0.5 - xoff) / day_width);
        if (start_day < 1)
            start_day = 1;
        else if (start_day > 7)
            start_day = 7;

        int end_day = start_day + (item->end_day - item->start_day);
        if (end_day < 1)
            end_day = 1;
        else if (end_day > 7)
            end_day = 7;
        itemMoved(item, start, end, start_day, end_day);
    }

    bool WeekScene::validMove(ScheduleItem* item, const QPointF& np)
    {
        if (!schedule)
            return true;

        QTime start = yToTime(np.y());
        int d = item->start.secsTo(item->end); // duration in seconds
        QTime end = start.addSecs(d);

        int start_day = 1 + floor((np.x() + day_width * 0.5 - xoff) / day_width);
        int end_day = start_day + (item->end_day - item->start_day);
        if (end_day > 7)
            end_day = 7;
        return schedule->validModify(item, start, end, start_day, end_day);
    }


    void WeekScene::itemResized(ScheduleItem* item, const QRectF& r)
    {
        QTime start = yToTime(r.y());
        QTime end = yToTime(r.y() + r.height());

        int start_day = 1 + floor((r.x() + day_width * 0.5 - xoff) / day_width);
        int end_day = 1 + floor((r.x() + r.width() - day_width * 0.5 - xoff) / day_width);
        if (start_day < 1)
            start_day = 1;
        else if (start_day > 7)
            start_day = 7;

        if (end_day < 1)
            end_day = 1;
        else if (end_day > 7)
            end_day = 7;

        itemMoved(item, start, end, start_day, end_day);
    }

    bool WeekScene::validResize(ScheduleItem* item, const QRectF& r)
    {
        QTime start = yToTime(r.y());
        QTime end = yToTime(r.y() + r.height());
        return schedule->validModify(item, start, end, item->start_day, item->end_day);
    }


    void WeekScene::itemChanged(ScheduleItem* item, QGraphicsItem* gi)
    {
        ScheduleGraphicsItem* sgi = (ScheduleGraphicsItem*)gi;
        qreal x = xoff + (item->start_day - 1) * day_width;
        qreal y = timeToY(item->start);
        qreal ye = timeToY(item->end);
        sgi->update(QRectF(x, y, day_width * (item->end_day - item->start_day + 1), ye - y));
    }

    void WeekScene::colorsChanged()
    {
        QPen pen(SchedulerPluginSettings::scheduleLineColor());
        QBrush brush(SchedulerPluginSettings::scheduleBackgroundColor());

        for (QGraphicsLineItem* line : qAsConst(lines))
            line->setPen(pen);

        for (QGraphicsRectItem* rect : qAsConst(rects))
        {
            rect->setPen(pen);
            rect->setBrush(brush);
        }

        pen.setStyle(Qt::DashLine);
        gline[0]->setPen(pen);
        gline[1]->setPen(pen);
    }

    void WeekScene::setShowGuidanceLines(bool on)
    {
        gline[0]->setVisible(on);
        gline[1]->setVisible(on);
    }

    void WeekScene::updateGuidanceLines(qreal y1, qreal y2)
    {
        const QString FORMAT = QStringLiteral("hh:mm");
        gline[0]->update(xoff, y1, yToTime(y1).toString(FORMAT));
        gline[1]->update(xoff, y2, yToTime(y2).toString(FORMAT));
    }
}

