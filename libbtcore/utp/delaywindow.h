/***************************************************************************
 *   Copyright (C) 2010 by Joris Guisson                                   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/


#ifndef UTP_DELAYWINDOW_H
#define UTP_DELAYWINDOW_H

#include <QPair>
#include <utp/utpprotocol.h>
#include <deque>


namespace utp 
{
	const bt::Uint32 MAX_DELAY = 0xFFFFFFFF;

	class BTCORE_EXPORT DelayWindow
	{
	public:
		DelayWindow();
		virtual ~DelayWindow();
		
		/// Update the window with a new packet, returns the base delay
		bt::Uint32 update(const Header* hdr,bt::TimeStamp receive_time);
		
		/// Get the current base delay
		bt::Uint32 baseDelay() const 
		{
			return lowest ? lowest->timestamp_difference_microseconds : MAX_DELAY;
		}
		
	private:
		struct DelayEntry
		{
			bt::Uint32 timestamp_difference_microseconds;
			bt::TimeStamp receive_time;
			
			DelayEntry() : timestamp_difference_microseconds(0),receive_time(0)
			{}
			
			DelayEntry(bt::Uint32 tdm,bt::TimeStamp rt) : timestamp_difference_microseconds(tdm),receive_time(rt)
			{}
			
			bool operator < (const DelayEntry & e) const {return receive_time < e.receive_time;}
		};
		
		typedef std::deque<DelayEntry>::iterator DelayEntryItr;
		
		DelayEntry* findBaseDelay();
		
	private:
		std::deque<DelayEntry> delay_window;
		DelayEntry* lowest;
	};

}

#endif // UTP_DELAYWINDOW_H
