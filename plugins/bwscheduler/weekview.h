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

#include <QGraphicsView>
#include <QMap>
#include <QMenu>

#include <util/constants.h>


namespace kt
{
    struct ScheduleItem;
    class Schedule;
    class WeekScene;

    /**
        Displays the schedule of one week.
    */
    class WeekView : public QGraphicsView
    {
        Q_OBJECT
    public:
        WeekView(QWidget* parent);
        ~WeekView();

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
        QList<ScheduleItem*> selectedItems() {return selection;}

        /**
         * Add an item to the schedule.
         * @param item The item to add
         */
        void addScheduleItem(ScheduleItem* item);


        /**
         * Remove all selected items from the schedule.
         */
        void removeSelectedItems();

        /// Get the right click menu
        QMenu* rightClickMenu() {return menu;}

        /**
         * Update the text of the status line
         * @param up Up speed
         * @param down Down speed
         * @param suspended Suspended or not
         * @param enabled Enabled or not
         */
        void updateStatusText(int up, int down, bool suspended, bool enabled);

        /**
         * Something has changed about an item
         * @param item
         */
        void itemChanged(ScheduleItem* item);

        /**
         * The color settings have changed.
         */
        void colorsChanged();

    Q_SIGNALS:
        void selectionChanged();
        void editItem(ScheduleItem* item);
        void itemMoved(ScheduleItem* item, const QTime& start, const QTime& end, int start_day, int end_day);

    private Q_SLOTS:
        void onSelectionChanged();
        void showContextMenu(const QPoint& pos);
        void onDoubleClicked(QGraphicsItem* i);

    private:
        WeekScene* scene;

        Schedule* schedule;
        QMap<QGraphicsItem*, ScheduleItem*>  item_map;
        QList<ScheduleItem*> selection;
        QMenu* menu;
    };

}

#endif
