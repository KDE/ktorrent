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

#include <interfaces/activity.h>

class QCheckBox;

namespace kt
{
    class WeekView;
    class Schedule;
    struct ScheduleItem;

    /**
        @author
    */
    class ScheduleEditor : public Activity
    {
        Q_OBJECT
    public:
        ScheduleEditor(QWidget* parent);
        ~ScheduleEditor();

        /**
         * Set the current Schedule
         * @param s The current schedule
         */
        void setSchedule(Schedule* s);

        /**
         * Update the text of the status line
         * @param up Up speed
         * @param down Down speed
         * @param suspended Suspended or not
         * @param enabled Enabled or not
         */
        void updateStatusText(int up, int down, bool suspended, bool enabled);

        /**
         * The color settings have changed
         */
        void colorsChanged();

    private slots:
        void clear();
        void save();
        void load();
        void addItem();
        void removeItem();
        void editItem();
        void onSelectionChanged();
        void editItem(ScheduleItem* item);
        void itemMoved(ScheduleItem* item, const QTime& start, const QTime& end, int start_day, int end_day);
        void enableChecked(bool on);

    signals:
        /**
         * Emitted when the user loads a new schedule.
         * @param ns The new schedule
         */
        void loaded(Schedule* ns);

        /**
         * Emitted when something changes in the schedule.
         */
        void scheduleChanged();

    private:
        void setupActions();
        QAction* addAction(const QString& icon, const QString& text, const QString& name, QObject* obj, const char* slot);

    private:
        WeekView* view;
        Schedule* schedule;

        QAction* load_action;
        QAction* save_action;
        QAction* new_item_action;
        QAction* remove_item_action;
        QAction* edit_item_action;
        QAction* clear_action;
        QCheckBox* enable_schedule;
    };

}

#endif
