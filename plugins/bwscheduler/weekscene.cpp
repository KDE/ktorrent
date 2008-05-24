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
#include <math.h>
#include <kglobal.h>
#include <klocale.h>
#include <kcalendarsystem.h>
#include <QFontMetricsF>
#include <QGraphicsRectItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <QGraphicsSceneMouseEvent>

#include <util/functions.h>
#include <util/log.h>
#include "weekscene.h"
#include "schedule.h"
#include "schedulegraphicsitem.h"

using namespace bt;

namespace kt
{

	WeekScene::WeekScene(QObject* parent)
			: QGraphicsScene(parent)
	{
		setBackgroundBrush(Qt::white);
		addCalendar();
	}


	WeekScene::~WeekScene()
	{}
	
	qreal LongestDayWidth(const QFontMetricsF & fm)
	{
		const KCalendarSystem* cal = KGlobal::locale()->calendar();
		qreal wd = 0;
		for (int i = 1;i <= 7;i++)
		{
			qreal w = fm.width(cal->weekDayName(i));
			if (w > wd)
				wd = w;
		}
		return wd;
	}
	
	void WeekScene::updateStatusText(int up,int down,bool paused)
	{
		if (paused)
			status->setPlainText(i18n("Current schedule: paused"));
		else if (up > 0 && down > 0)
			status->setPlainText(i18n("Current schedule: %1 KB/s download, %2 KB/s upload",down,up));
		else if (up > 0)
			status->setPlainText(i18n("Current schedule: unlimited download, %1 KB/s upload",up));
		else if (down > 0)
			status->setPlainText(i18n("Current schedule: %1 KB/s download, unlimited upload",down));
		else
			status->setPlainText(i18n("Current schedule: unlimited upload and download"));
	}

	void WeekScene::addCalendar()
	{
		const KCalendarSystem* cal = KGlobal::locale()->calendar();
		
		QGraphicsTextItem* tmp = addText("Dinges");
		QFontMetricsF fm(tmp->font());
		removeItem(tmp);
		delete tmp;
		
		
		// first add 7 rectangles for each day of the week
		xoff = fm.width("00:00") + 10;
		yoff = 2*fm.height() + 10;
		day_width = LongestDayWidth(fm) * 1.5;
		hour_height = fm.height() * 1.5;
		
		status = addText(i18n("Current schedule:"));
		status->setPos(QPointF(0,0));
		status->setZValue(2);
		
		for (int i = 0;i < 7;i++)
		{
			QGraphicsRectItem* item = addRect(xoff + day_width * i,yoff,day_width,24 * hour_height,QPen(Qt::blue),QBrush(Qt::yellow));
			item->setZValue(1);
			
			QString day = cal->weekDayName(i+1);
			
			// make sure day is centered in the middle of the column 
			qreal dlen = fm.width(day);
			qreal mid = xoff + day_width * (i + 0.5);
			qreal start = mid - dlen * 0.5;
			
			QGraphicsTextItem* t = addText(day);
			t->setPos(QPointF(start, fm.height() + 5));
			t->setZValue(2);
		}
		
		// draw hour lines
		for (int i = 0;i <= 24;i++)
		{
			QGraphicsLineItem* item = addLine(0, yoff + i*hour_height,xoff + 7*day_width, yoff + i*hour_height,QPen(Qt::blue));
			item->setZValue(2);
			
			if (i < 24)
			{
				QGraphicsTextItem* t = addText(QString("%1:00").arg(i));
				t->setPos(QPointF(0, yoff + i * hour_height));
				t->setZValue(2);
			}
		}
		
		QRectF r = sceneRect();
		r.setHeight(r.height() + 10);
		setSceneRect(r);
	}
	
	QGraphicsItem* WeekScene::addScheduleItem(ScheduleItem* item)
	{
		QTime midnight(0,0,0,0);
		qreal x = xoff + (item->day - 1) * day_width;
		qreal min_h = hour_height / 60.0;
		qreal y = timeToY(item->start);
		qreal ye = timeToY(item->end);
		
		QRectF rect(x,y,day_width,ye - y);
		QRectF cst(x,yoff,day_width,24*hour_height); 
		ScheduleGraphicsItem* gi = new ScheduleGraphicsItem(item,rect,cst,this);
		addItem(gi);
		gi->update(rect,cst);
		return gi;
	}
	
	void WeekScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* ev)
	{
		QList<QGraphicsItem*> gis = items(ev->scenePos());	
		foreach (QGraphicsItem* gi,gis)
		{
			if (gi->zValue() == 3)
			{
				itemDoubleClicked(gi);
				break;
			}
		}
	}
	/*
	void WeekScene::mouseMoveEvent(QGraphicsSceneMouseEvent* ev)
	{
	}
	*/
	void WeekScene::mousePressEvent(QGraphicsSceneMouseEvent* ev)
	{
		if (ev->button() == Qt::RightButton)
		{
			QList<QGraphicsItem*> gis = items(ev->scenePos());	
			foreach (QGraphicsItem* gi,gis)
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
	
	qreal WeekScene::timeToY(const QTime & time)
	{
		QTime midnight(0,0,0,0);
		qreal min_h = hour_height / 60.0; 
		return (midnight.secsTo(time) / 60.0) * min_h + yoff;
	}
	
	QTime WeekScene::yToTime(qreal y)
	{
		y = y - yoff; // get rid of offset
		qreal min_h = hour_height / 60.0; 
		return QTime(0,0,0,0).addSecs((y / min_h) * 60);
	}
	
	void WeekScene::itemMoved(ScheduleItem* item,const QPointF & np)
	{
		QTime start = yToTime(np.y());		
		int d = item->start.secsTo(item->end); // duration in seconds
		QTime end = start.addSecs(d);
		end = end.addSecs(59 - end.second()); // make sure end seconds is 59
		
		itemMoved(item,start,end);
	}
	
	void WeekScene::itemChanged(ScheduleItem* item,QGraphicsItem* gi)
	{
		ScheduleGraphicsItem* sgi = (ScheduleGraphicsItem*)gi;
		qreal x = xoff + (item->day - 1) * day_width;
		qreal y = timeToY(item->start);
		qreal ye = timeToY(item->end);
		sgi->update(QRectF(x,y,day_width,ye - y),QRectF(x,yoff,day_width,24*hour_height));
	}
}

#include "weekscene.moc"
