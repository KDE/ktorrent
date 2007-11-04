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
#ifndef KTWEEKVIEW_H
#define KTWEEKVIEW_H

#include <QMap>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <util/constants.h>


namespace kt
{
	struct ScheduleItem;
	class Schedule;

	/**
		Displays the schedule of one week.
	*/
	class WeekView : public QGraphicsView
	{
		Q_OBJECT
	public:
		WeekView(QWidget* parent);
		virtual ~WeekView();
		
		/**
		 * Set the current Schedule
		 * @param s The current schedule
		 */
		void setSchedule(Schedule* s);
		
		/**
		 * Clear the current Schedule.
		 */
		void clear();
		
		/// Get the selected items
		QList<ScheduleItem> selectedItems() {return selection;}
		
		/**
		 * Add an item to the schedule.
		 * @param item The item to add
		 */
		void addScheduleItem(const ScheduleItem & item);
		
		
		/**
		 * Remove all selected items from the schedule.
		 */
		void removeSelectedItems();
		
		
	signals:
		void selectionChanged();
		
	private slots:
		void onSelectionChanged();
		
	private:
		void addCalendar();
		
	private:
		QGraphicsScene scene;
		qreal xoff;
		qreal yoff;
		qreal day_width;
		qreal hour_height;
		Schedule* schedule;
		QMap<QGraphicsItem*,ScheduleItem>  item_map;
		QList<ScheduleItem> selection;
	};

}

#endif
