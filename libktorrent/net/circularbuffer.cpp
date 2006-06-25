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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include <util/log.h>
#include <torrent/globals.h>
#include "circularbuffer.h"
#include "bufferedsocket.h"

using namespace bt;

namespace net
{

	CircularBuffer::CircularBuffer(Uint32 max_size) : buf(0),max_size(max_size),first(0),size(0)
	{
		buf = new Uint8[max_size];
	}


	CircularBuffer::~CircularBuffer()
	{
		delete [] buf;
	}
	
	Uint32 CircularBuffer::freeSpace() const
	{
		return max_size - size;
	}

	Uint32 CircularBuffer::write(const Uint8* data,Uint32 dsize)
	{
		if (size == max_size)
			return 0;
		
		mutex.lock();
		Uint32 wp = (first + size) % max_size;
		Uint32 j = 0;
		while (size < max_size && (dsize == 0 || j < dsize))
		{
			buf[wp] = data[j];
			j++;
			wp = (wp + 1) % max_size;
			size++;
		} 
		
		mutex.unlock();
		return j;
	}
	
	Uint32 CircularBuffer::read(Uint8* data,Uint32 max_to_read)
	{
		if (!size)
			return 0;
		
		mutex.lock();
		Uint32 j = 0;
		while (size > 0 && j < max_to_read)
		{
			data[j] = buf[first];
			j++;
			first = (first + 1) % max_size;
			size--;
		}
		mutex.unlock();
		return j;
	}
	
	Uint32 CircularBuffer::send(BufferedSocket* s,Uint32 max)
	{
		if (!size)
			return 0;
		
		Uint32 ret = 0;
		mutex.lock();
		
		if (first + size <= max_size)
		{
			Uint32 ts = size;
			if (max > 0 && size > max)
				ts = max;
			ret = s->send(buf + first,ts);
			first += ret;
			size -= ret;
		}
		else if (max > 0) // if there is a limit
		{
			// write from first to the end of the buffer
			Uint32 to_send = max_size - first;
			if (to_send > max)
				to_send = max;
				
			ret = s->send(buf + first,to_send);
		
			// update first, wrap around if necessary
			first = (first + ret) % max_size;
			size -= ret; // ret bytes less in the buffer
			max -= ret; // decrease limit
		
			if (max > 0 && ret == to_send && size > 0)
			{
				// we have sent everything so we can send more
				to_send = size > max ? max : size;
				Uint32 ret2 = s->send(buf,to_send);
				
				ret += ret2;
				first += ret2;
				size -= ret2;
			}
		}
		else // no limit
		{
			Uint32 to_send = max_size - first;
			ret = s->send(buf + first,to_send);
			// update first, wrap around if necessary
			first = (first + ret) % max_size;
			size -= ret; // ret bytes less in the buffer
			if (ret == to_send && size > 0)
			{
				// we have sent everything so we can send more
				Uint32 ret2 = s->send(buf,size);
				ret += ret2;
				first += ret2;
				size -= ret2;
			}
		}
		mutex.unlock();
		return ret;
	}

}
