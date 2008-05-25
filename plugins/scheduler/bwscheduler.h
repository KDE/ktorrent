/***************************************************************************
 *   Copyright (C) 2006 by Ivan Vasić                                      *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.           *
 ***************************************************************************/
#ifndef KTBWSCHEDULER_H
#define KTBWSCHEDULER_H

#include <interfaces/coreinterface.h>

namespace kt
{
	
	typedef enum bws_category
	{
		CAT_NORMAL,
		CAT_FIRST,
		CAT_SECOND,
		CAT_THIRD,
		CAT_OFF
	}ScheduleCategory;
	
	/**
	 * @author Ivan Vasic <ivasic@gmail.com>
	 * @brief This class represents bandwidth schedule for the week.
	 * It simplifies usage of 2 dimensional schedule (array) and its transfer between classes.
	 */
	class BWS
	{
		///Schedule
		ScheduleCategory** m_schedule;
									   
		///Download categories
		int download[3];
		///Upload categories
		int upload[3];
		
		public:
			BWS();
			BWS& operator=(const BWS& b);
			~BWS();
			
			/**
			 * @brief Resets this schedule.
			 */
			void reset();
			
			
			/**
			 * Gets download rate for category <i>cat</i>.
			 * @param cat download category index
			 * @return Download rate in KB/s
			 */
			int getDownload(int cat);
			
			/**
			 * Gets upload rate for category <i>cat</i>.
			 * @param cat upload category index
			 * @return upload rate in KB/s
			 */
			int getUpload(int cat);
			
			
			/**
			 * @brief Gets category for specified <i>day</i> and <i>hour</i>.
			 * @param day Number of day in a week.
			 * @param hour Hour of the day.
			 * @return ScheduleCategory - category associated with that day/hour.
			 */
			ScheduleCategory getCategory(int day, int hour);
			
			
			/**
			 * @brief Sets download rate for a category.
			 * @param cat Category to set rate to.
			 * @param val Download rate to set to category in KB/s
			 */
			void setDownload(int cat, int val);
			
			/**
			 * @brief Sets upload rate for a category.
			 * @param cat Category to set rate to.
			 * @param val Upload rate to set to category in KB/s
			 */
			void setUpload(int cat, int val);
			
			
			/**
			 * @brief Sets category for specified <i>day</i>/<i>hour</i> combination.
			 * @param day Day of the week.
			 * @param hour Hour of the day.
			 * @param val Category value.
			 */
			void setCategory(int day, int hour, ScheduleCategory val);
			
			///Prints schedule to LogViewer. Used only for debugging.
			void debug();
	};
	
	
	/**
	 * @brief Bandwidth scheduler class.
	 * @author Ivan Vasic <ivasic@gmail.com>
	 * Singleton class. Used to keep bandwidth schedule and change download/upload rate as necessary.
	 */
	class BWScheduler
	{
		public:
			inline static BWScheduler& instance()
			{
				static BWScheduler self;
				return self;
			}
			~BWScheduler();
			
			/**
			 * Triggers bandwidth limit changes (if needed).
			 */
			void trigger();
			
			/**
			 * Sets a new schedule.
			 * @param sch - new BWS schedule.
			 * @note Call trigger() after setting new schedule for changes to take effect.
			 */
			void setSchedule(const BWS& sch);
			
			/**
			 * Sets a pointer to CoreInterface.
			 * Needed for getting global bandwidth limits.
			 * @param core Pointer to CoreInterface
			 */
			void setCoreInterface(CoreInterface* core);
			
			///Pauses all torrents (TURN OFF category)
			void pauseAll();
			
			///Loads schedule from HD
			void loadSchedule();
			///Saves schedule to HD
			void saveSchedule();

	void setEnabled(bool theValue);
	
			
		protected:
			BWScheduler();
			BWScheduler(const BWScheduler&);
			BWScheduler& operator=(const BWScheduler&);
			
			BWS m_schedule;
			CoreInterface* m_core;
			
			bool m_enabled;
	};
}

#endif

