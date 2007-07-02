/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#ifndef BTCAP_H
#define BTCAP_H

#if 0
#include <qvaluelist.h>
#include <util/timer.h>
#include <util/constants.h>

namespace bt
{
	/**
	 * Base class for all cappable objects.
	*/
	class Cappable
	{
	public:
		/**
		 * Proceed with doing some bytes
		 * @param bytes The number of bytes it can do (0 = no limit)
		 * @return true if finished, false otherwise
		 */
		virtual void proceed(Uint32 bytes) = 0;
	};

	/**
	 * @author Joris Guisson <joris.guisson@gmail.com>
	 * 
	 * A Cap is something which caps something.
	*/
	class Cap
	{
	public:
		Cap(bool percentage_check);
		virtual ~Cap();

		struct Entry
		{
			Cappable* obj;
			Uint32 num_bytes;
			
			Entry() : obj(0),num_bytes(0) {}
			Entry(Cappable* obj,Uint32 nb) : obj(obj),num_bytes(nb) {}
		};
		
		/**
		 * Set the speed cap in bytes per second. 0 indicates
		 * no limit.
		 * @param max Maximum number of bytes per second.
		 */
		void setMaxSpeed(Uint32 max);
		
		/// Get max bytes/sec
		Uint32 getMaxSpeed() const {return max_bytes_per_sec;}
		
		/// Set the current speed
		void setCurrentSpeed(Uint32 cs) {current_speed = cs;}
		
		/// Get the current speed
		Uint32 getCurrrentSpeed() const {return current_speed;}
		
		/**
		 * Allow or disallow somebody from proceeding. If somebody
		 * is disallowed they will be stored in a queue, and will be notified
		 * when there turn is up.
		 * @param pd Thing which is doing the request
		 * @param bytes Bytes it wants to send
		 * @return true if the piece is allowed or not
		 */
		bool allow(Cappable* pd,Uint32 bytes);

		/**
		 * A thing in the queue should call this when it get destroyed. To
		 * remove them from the queue.
		 * @param pd The Cappable thing
		 */
		void killed(Cappable* pd);

		/**
		 * Update the downloadcap.
		 */
		void update();
		
	private:
		QValueList<Entry> entries;
		Uint32 max_bytes_per_sec;
		Timer timer;
		Uint32 leftover;
		Uint32 current_speed;
		bool percentage_check;
	};

}
#endif
#endif
