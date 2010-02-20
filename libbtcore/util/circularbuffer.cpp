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

#include "circularbuffer.h"
#include <string.h>
#include "log.h"


namespace bt
{
	
	CircularBuffer::CircularBuffer(Uint32 cap) : boost::circular_buffer<bt::Uint8>(cap)
	{
	}

	CircularBuffer::~CircularBuffer()
	{
	}

	bt::Uint32 CircularBuffer::read(bt::Uint8* data, bt::Uint32 max_len)
	{
		if (size() == 0)
			return 0;
		
		bt::Uint32 bs = size();
		bt::Uint32 to_read = bs < max_len ? bs : max_len;
		bt::Uint32 i = 0;
		while (i < to_read)
		{
			data[i] = front();
			pop_front();
			i++;
		}
		
		return to_read;
	}
	
	bt::Uint32 CircularBuffer::write(const bt::Uint8* data, bt::Uint32 len)
	{
		if (full())
			return 0;
		
		bt::Uint32 free_space = capacity() - size();
		bt::Uint32 to_write = free_space < len ? free_space : len;
		for (bt::Uint32 i = 0;i < to_write;i++)
			push_back(data[i]);
		
		return to_write;
	}
	
}

