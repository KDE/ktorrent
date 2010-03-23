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

#include "delaywindow.h"
#include <algorithm>
#include <util/functions.h>

namespace utp
{
	
	DelayWindow::DelayWindow() : lowest(0)
	{
	}
	
	DelayWindow::~DelayWindow()
	{

	}
	
	bt::Uint32 DelayWindow::update(const utp::Header* hdr,bt::TimeStamp receive_time)
	{
		bt::TimeStamp now = receive_time;
		
		// drop everything older then 2 minutes
		DelayEntryItr itr = delay_window.begin();
		while (itr != delay_window.end())
		{
			if (now - itr->receive_time > DELAY_WINDOW_SIZE)
			{
				if (lowest == &(*itr))
					lowest = 0;
				
				itr = delay_window.erase(itr);
			}
			else
				break;
		}
		
		if (!lowest)
			lowest = findBaseDelay();
		
		// Add the new entry and check if it updates base delay
		DelayEntry entry(hdr->timestamp_difference_microseconds,now);
		delay_window.push_back(entry);
		if (!lowest || entry.timestamp_difference_microseconds < lowest->timestamp_difference_microseconds)
			lowest = &delay_window.back();
		
		return lowest->timestamp_difference_microseconds;
	}
	
	DelayWindow::DelayEntry* DelayWindow::findBaseDelay()
	{
		bt::Uint32 base_delay = MAX_DELAY;
		DelayEntryItr found = delay_window.end();
		DelayEntryItr itr = delay_window.begin();
		while (itr != delay_window.end())
		{
			if (itr->timestamp_difference_microseconds < base_delay)
			{
				base_delay = itr->timestamp_difference_microseconds;
				found = itr;
			}
			itr++;
		}
		
		if (found == delay_window.end())
			return 0;
		else
			return &(*found);
	}

}

