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

#include <util/log.h>
#include <QGraphicsItem>
#include "weekview.h"
#include "weekscene.h"
#include "schedule.h"

using namespace bt;

namespace kt
{

	WeekView::WeekView(QWidget* parent) : QGraphicsView(parent),schedule(0)
	{
		scene = new WeekScene(this);
		setScene(scene);
		
		connect(scene,SIGNAL(selectionChanged()),this,SLOT(onSelectionChanged()));
		connect(scene,SIGNAL(itemDoubleClicked(QGraphicsItem*)),this,SLOT(onDoubleClicked(QGraphicsItem*)));
		
		menu = new KMenu(this);
		setContextMenuPolicy(Qt::CustomContextMenu);
		connect(this,SIGNAL(customContextMenuRequested(const QPoint & )),
				this,SLOT(showContextMenu(const QPoint& )));
	}


	WeekView::~WeekView()
	{
	}
	
	void WeekView::updateStatusText(int up,int down,bool paused)
	{
		scene->updateStatusText(up,down,paused);
	}
	
	void WeekView::onSelectionChanged()
	{
		selection.clear();
		
		QList<QGraphicsItem*> sel = scene->selectedItems();
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
			scene->removeItem(item);
			delete item;
			i++;
		}
		item_map.clear();
		selection.clear();
		schedule = 0;
	}
	
	void WeekView::removeSelectedItems()
	{
		QList<QGraphicsItem*> sel = scene->selectedItems();
		foreach (QGraphicsItem* s,sel)
		{
			QMap<QGraphicsItem*,ScheduleItem>::iterator i = item_map.find(s);
			if (i != item_map.end())
			{
				const ScheduleItem & si = i.value();
				schedule->removeAll(si);
				scene->removeItem(s);
				item_map.erase(i);
				delete s;
			}
		}
	}
	
	void WeekView::addScheduleItem(const ScheduleItem & item)
	{
		QGraphicsItem* gi = scene->addScheduleItem(item);
		
		if (gi)
			item_map[gi] = item;
	}
	
	void WeekView::onDoubleClicked(QGraphicsItem* i)
	{
		QMap<QGraphicsItem*,ScheduleItem>::iterator itr = item_map.find(i);
		if (itr != item_map.end())
			editItem(itr.value());
	}
	
	void WeekView::showContextMenu(const QPoint& pos)
	{
		menu->popup(mapToGlobal(pos));
	}
}

#include "weekview.moc"