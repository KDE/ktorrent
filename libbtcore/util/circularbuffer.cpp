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


namespace bt
{
	
	CircularBuffer::CircularBuffer(Uint32 cap) : window(0),buffer_capacity(cap),start(0),size(0)
	{
		window = new bt::Uint8[buffer_capacity];
	}

	CircularBuffer::~CircularBuffer()
	{
		delete [] window;
	}

	bt::Uint32 CircularBuffer::read(bt::Uint8* data, bt::Uint32 max_len)
	{
		if (size == 0)
			return 0;
		
		bt::Uint32 to_read = size < max_len ? size : max_len;
		if (start + to_read < buffer_capacity)
		{
			// we are not going past the end of the data
			memcpy(data,window + start,to_read);
			start += to_read;
			size -= to_read;
			return to_read;
		}
		else
		{
			// read until the end of the window
			memcpy(data,window + start,buffer_capacity - start);
			bt::Uint32 ar = buffer_capacity - start;
			if (to_read > ar) // read the rest
				memcpy(data + ar,window,to_read - ar);
			
			start = ar;
			size -= to_read;
			return to_read;
		}
	}
	
	bt::Uint32 CircularBuffer::write(const bt::Uint8* data, bt::Uint32 len)
	{
		if (size == buffer_capacity)
			return 0;
		
		bt::Uint32 free_space = buffer_capacity - size;
		bt::Uint32 to_write = free_space < len ? free_space : len;
		bt::Uint32 off = (start + size) % buffer_capacity;
		if (off + to_write < buffer_capacity)
		{
			// everything will go in one go
			memcpy(window + off,data,to_write);
			size += to_write;
			return to_write;
		}
		else
		{
			memcpy(window + off,data,buffer_capacity - off);
			bt::Uint32 aw = buffer_capacity - off;
			if (to_write > aw)
				memcpy(window,data + aw,to_write - aw);
			
			size += to_write;
			return to_write;
		}
	}
}

