/***************************************************************************
 *   Copyright (C) 2006 by Ivan Vasić                                      *
 *   ivan@ktorrent.org                                                     *
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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.           *
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
	 */
	class BWS
	{	
		ScheduleCategory** m_schedule;
									   
		int download[3];
		int upload[3];
		
		public:
			BWS();
			BWS& operator=(const BWS& b);
			~BWS();
			
			void reset();
			
			int getDownload(int cat);
			int getUpload(int cat);
			ScheduleCategory getCategory(int day, int hour);
			
			void setDownload(int cat, int val);
			void setUpload(int cat, int val);
			void setCategory(int day, int hour, ScheduleCategory val);
			
			void debug();
	};
	
	
	/**
	 * @brief Bandwidth scheduler class.
	 * @author Ivan Vasic <ivan@ktorrent.org>
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
			
			/**
			 * Pauses all torrents (TURN OFF category)
			 */
			void pauseAll();
			
			///Loads schedule from HD
			void loadSchedule();
			///Saves schedule to HD
			void saveSchedule();
			
		protected:
			BWScheduler();
			BWScheduler(const BWScheduler&);
			BWScheduler& operator=(const BWScheduler&);
			
			BWS m_schedule;
			CoreInterface* m_core;
	};
}

#endif

