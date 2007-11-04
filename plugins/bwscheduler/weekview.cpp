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
#include <QGraphicsRectItem>
#include <QGraphicsLineItem>
#include <QGraphicsTextItem>
#include <util/log.h>
#include "weekview.h"
#include "schedule.h"

using namespace bt;

namespace kt
{

	WeekView::WeekView(QWidget* parent) : QGraphicsView(parent),schedule(0)
	{
		addCalendar();
		setScene(&scene);
		scene.setBackgroundBrush(Qt::white);
		connect(&scene,SIGNAL(selectionChanged()),this,SLOT(onSelectionChanged()));
	}


	WeekView::~WeekView()
	{
	}
	
	void WeekView::onSelectionChanged()
	{
		selection.clear();
		
		QList<QGraphicsItem*> sel = scene.selectedItems();
		foreach (QGraphicsItem* s,sel)
		{
			QMap<QGraphicsItem*,ScheduleItem>::iterator i = item_map.find(s);
			if (i != item_map.end())
				selection.append(i.value());
		}
		
		selectionChanged();
	}
	
	void WeekView::setSchedule(Schedule* s)
	{
		clear();
		schedule = s;
		
		if (schedule)
		{
			foreach (ScheduleItem i,*schedule)
				addScheduleItem(i);
		}
	}
		
	void WeekView::clear()
	{
		QMap<QGraphicsItem*,ScheduleItem>::iterator i = item_map.begin();
		while (i != item_map.end())
		{
			QGraphicsItem* item = i.key();
			scene.removeItem(item);
			delete item;
			i++;
		}
		item_map.clear();
		selection.clear();
		schedule = 0;
	}
	
	void WeekView::removeSelectedItems()
	{
		QList<QGraphicsItem*> sel = scene.selectedItems();
		foreach (QGraphicsItem* s,sel)
		{
			QMap<QGraphicsItem*,ScheduleItem>::iterator i = item_map.find(s);
			if (i != item_map.end())
			{
				const ScheduleItem & si = i.value();
				schedule->removeAll(si);
				scene.removeItem(s);
				item_map.erase(i);
				delete s;
			}
		}
	}
	
	void WeekView::addScheduleItem(const ScheduleItem & item)
	{
		Out(SYS_SCD|LOG_DEBUG) << "WeekView::addScheduleItem" << endl;
		Out(SYS_SCD|LOG_DEBUG) << "day = " << item.day << endl;
		Out(SYS_SCD|LOG_DEBUG) << "start = " << item.start.toString() << endl;
		Out(SYS_SCD|LOG_DEBUG) << "end = " << item.end.toString() << endl;
		Out(SYS_SCD|LOG_DEBUG) << "upload_limit = " << item.upload_limit << endl;
		Out(SYS_SCD|LOG_DEBUG) << "download_limit = " << item.download_limit << endl;
		Out(SYS_SCD|LOG_DEBUG) << "paused = " << item.paused << endl;
		
		QTime midnight(0,0,0,0);
		qreal x = xoff + (item.day - 1) * day_width;
		qreal min_h = hour_height / 60.0;
		qreal y = yoff + (midnight.secsTo(item.start) / 60.0) * min_h;
		qreal ye = yoff + (midnight.secsTo(item.end) / 60.0) * min_h;
		
		Out(SYS_SCD|LOG_DEBUG) << "Pos: " << x << " " << y << " " << ye << " " << min_h << endl;
		
		QGraphicsRectItem* gi = scene.addRect(x,y,day_width,ye - y);
		gi->setPen(QPen(Qt::black));
		gi->setZValue(3);
		QBrush brush(QColor(0,255,0,125));
		gi->setBrush(brush);
		gi->setFlag(QGraphicsItem::ItemIsSelectable,true);
		
		item_map[gi] = item;
	}
	
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

	void WeekView::addCalendar()
	{
		const KCalendarSystem* cal = KGlobal::locale()->calendar();
		
		QGraphicsTextItem* tmp = scene.addText("Dinges");
		QFontMetricsF fm(tmp->font());
		scene.removeItem(tmp);
		delete tmp;
		
		// first add 7 rectangles for each day of the week
		xoff = fm.width("00:00") + 10;
		yoff = fm.height() + 5;
		day_width = LongestDayWidth(fm) * 1.5;
		hour_height = fm.height() * 1.5;
		
		for (int i = 0;i < 7;i++)
		{
			QGraphicsRectItem* item = scene.addRect(xoff + day_width * i,yoff,day_width,24 * hour_height,QPen(Qt::blue),QBrush(Qt::yellow));
			item->setZValue(1);
			
			QString day = cal->weekDayName(i+1);
			
			// make sure day is centered in the middle of the column 
			qreal dlen = fm.width(day);
			qreal mid = xoff + day_width * (i + 0.5);
			qreal start = mid - dlen * 0.5;
			
			QGraphicsTextItem* t = scene.addText(day);
			t->setPos(QPointF(start, 0));
			t->setZValue(2);
		}
		
		// draw hour lines
		for (int i = 0;i <= 24;i++)
		{
			QGraphicsLineItem* item = scene.addLine(0, yoff + i*hour_height,xoff + 7*day_width, yoff + i*hour_height,QPen(Qt::blue));
			item->setZValue(2);
			
			if (i < 24)
			{
				QGraphicsTextItem* t = scene.addText(QString("%1:00").arg(i));
				t->setPos(QPointF(0, yoff + i * hour_height));
				t->setZValue(2);
			}
		}
	}

}

#include "weekview.moc"