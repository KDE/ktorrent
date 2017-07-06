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

#ifndef KTSCHEDULE_H
#define KTSCHEDULE_H

#include <QList>
#include <QTime>
#include <util/constants.h>

namespace bt
{
    class BDictNode;
    class BListNode;
}

namespace kt
{
    template<class T>
    bool between(T v, T min_val, T max_val)
    {
        return v >= min_val && v <= max_val;
    }

    struct ScheduleItem
    {
        int start_day;
        int end_day;
        QTime start;
        QTime end;
        bt::Uint32 upload_limit;
        bt::Uint32 download_limit;
        bool suspended;
        bool screensaver_limits;
        bt::Uint32 ss_upload_limit;
        bt::Uint32 ss_download_limit;
        bool set_conn_limits;
        bt::Uint32 global_conn_limit;
        bt::Uint32 torrent_conn_limit;

        ScheduleItem();
        ScheduleItem(const ScheduleItem& item);

        bool isValid() const
        {
            return
                between(start_day, 1, 7) &&
                between(end_day, 1, 7) &&
                start_day <= end_day;
        }

        /**
         * Check if this item conflicts with another
         * @param other The other
         * @return true If there is a conflict, false otherwise
         */
        bool conflicts(const ScheduleItem& other) const;

        /**
         * Assignment operator
         * @param item The item to copy
         * @return this
         */
        ScheduleItem& operator = (const ScheduleItem& item);

        /**
         * Comparison operator.
         * @param item Item to compare
         * @return true if the items are the same
         */
        bool operator == (const ScheduleItem& item) const;

        /// Whether or not a QDateTime is falls within this item
        bool contains(const QDateTime& dt) const;

        /// Check if start and end time are OK
        void checkTimes();
    };

    /**
     * Class which holds the schedule of one week.
    */
    class Schedule
    {
    public:
        Schedule();
        ~Schedule();

        /**
         * Load a schedule from a file.
         * This will clear the current schedule.
         * @param file The file to load from
         * @throw Error When this fails
         */
        void load(const QString& file);


        /**
         * Save a schedule to a file.
         * @param file The file to write to
         * @throw Error When this fails
         */
        void save(const QString& file);

        /**
         * Add a ScheduleItem to the schedule
         * @param item The ScheduleItem
         * @return true upon succes, false otherwise (probably conflicts with other items)
         */
        bool addItem(ScheduleItem* item);

        /**
         * Get the current schedule item we should be setting.
         * @return 0 If the current time doesn't fall into any item, the item otherwise
         */
        ScheduleItem* getCurrentItem(const QDateTime& now);

        /**
         * Get the time in seconds to the next time we need to update the schedule.
         */
        int getTimeToNextScheduleEvent(const QDateTime& now);

        /**
         * Try to modify start, stop time and day of an item.
         * @param item The item
         * @param start The start time
         * @param end The stop time
         * @param start_day The start day
         * @param end_day The end day
         * @return true If this succeeds (i.e. no conflicts)
         */
        bool modify(ScheduleItem* item, const QTime& start, const QTime& end, int start_day, int end_day);

        /**
         * Would a modify succeed ?
         * @param item The item
         * @param start The start time
         * @param end The stop time
         * @param start_day The start day
         * @param end_day The end day
         * @return true If this succeeds (i.e. no conflicts)
         */
        bool validModify(ScheduleItem* item, const QTime& start, const QTime& end, int start_day, int end_day);

        /**
         * Check for conflicts with other schedule items.
         * @param item The item
         */
        bool conflicts(ScheduleItem* item);

        /**
         * Disable or enabled the schedule
         */
        void setEnabled(bool on);

        /// Is the schedule enabled
        bool isEnabled() const {return enabled;}

        /// Clear the schedule
        void clear();

        /// Apply an operation on each ScheduleItem
        template<class Operation>
        void apply(Operation op)
        {
            for (ScheduleItem* i : qAsConst(items))
                op(i);
        }

        /// Remove a ScheduleItem, item will be deleted
        void removeItem(ScheduleItem* item);

        /// Get the number of items in the schedule
        int count() const {return items.count();}

    private:
        bool parseItem(ScheduleItem* item, bt::BDictNode* dict);
        void parseItems(bt::BListNode* items);

    private:
        bool enabled;
        QList<ScheduleItem*> items;
    };

}

#endif
