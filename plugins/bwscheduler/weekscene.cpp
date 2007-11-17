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
	}
	
	QGraphicsItem* WeekScene::addScheduleItem(const ScheduleItem & item)
	{
		QTime midnight(0,0,0,0);
		qreal x = xoff + (item.day - 1) * day_width;
		qreal min_h = hour_height / 60.0;
		qreal y = yoff + (midnight.secsTo(item.start) / 60.0) * min_h;
		qreal ye = yoff + (midnight.secsTo(item.end) / 60.0) * min_h;
		
		
		QGraphicsRectItem* gi = addRect(x,y,day_width,ye - y);
		gi->setPen(QPen(Qt::black));
		gi->setZValue(3);
		QString text;
		if (item.paused)
		{
			gi->setBrush(QBrush(QColor(255,0,0,255)));
			text = i18n("Paused");
		}
		else
		{
			gi->setBrush(QBrush(QColor(0,255,0,255)));
			text = i18n("%1 Down\n%2 Up",
						KBytesPerSecToString(item.download_limit),
											 KBytesPerSecToString(item.upload_limit));
		}
		gi->setFlag(QGraphicsItem::ItemIsSelectable,true);
		
		
		QGraphicsTextItem* t = addText(text);
		QFontMetricsF fm(t->font());
		
		t->setPos(QPointF(x, y));
		t->setZValue(4);
		t->setTextWidth(day_width);
		t->setParentItem(gi);
		gi->setToolTip(text);
		
		if (t->boundingRect().height() > gi->rect().height())
		{
			// Text is to big for rect
			delete t;
		}
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

}

#include "weekscene.moc"
