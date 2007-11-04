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
#ifndef KTSCHEDULEEDITOR_H
#define KTSCHEDULEEDITOR_H

#include <QWidget>

class KToolBar;

namespace kt
{
	class WeekView;
	class Schedule;

	/**
		@author
	*/
	class ScheduleEditor : public QWidget
	{
		Q_OBJECT
	public:
		ScheduleEditor(QWidget* parent);
		virtual ~ScheduleEditor();
		
		/**
		 * Set the current Schedule
		 * @param s The current schedule
		 */
		void setSchedule(Schedule* s);
		
	private slots:
		void clear();
		void save();
		void load();
		void addItem();
		void removeItem();
		void editItem();
		void onSelectionChanged();
		
	signals:
		/**
		 * Emitted when the user loads a new schedule.
		 * @param ns The new schedule
		 */
		void loaded(Schedule* ns);

	private:
		WeekView* view;
		Schedule* schedule;
		KToolBar* tool_bar;
		
		QAction* load_action;
		QAction* save_action;
		QAction* new_item_action;
		QAction* remove_item_action;
		QAction* edit_item_action;
		QAction* clear_action;
	};

}

#endif
