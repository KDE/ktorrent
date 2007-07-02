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
#ifndef NETCIRCULARBUFFER_H
#define NETCIRCULARBUFFER_H

#include <qmutex.h> 
#include <util/constants.h>

namespace net
{
	using bt::Uint8;
	using bt::Uint32;
	
	class BufferedSocket;

	/**
	 * @author Joris Guisson <joris.guisson@gmail.com>
	 * 
	 * Simple circular buffer, to simulate a queue.
	 * Writes happen at the end, reads at the beginning.
	 * The buffer is protected by a mutex.
	 */
	class CircularBuffer
	{
		Uint8* buf;
		Uint32 max_size;
		Uint32 first; // index of first byte in the buffer
		Uint32 size; // number of bytes in use
		mutable QMutex mutex;
	public:
		/**
		 * Create the buffer.
		 * @param max_size Maximum size of the buffer.
		 */
		CircularBuffer(Uint32 max_size);
		virtual ~CircularBuffer();

		/// How much capacity does the buffer have
		Uint32 capacity() const {return max_size;}
		
		/// How much free space is there 
		Uint32 freeSpace() const;
		
		
		/**
		 * Write a bunch of data at the back of the buffer.
		 * @param data Data to write
		 * @param size How many bytes to write
		 * @return The number of bytes written in the buffer
		 */
		Uint32 write(const Uint8* data,Uint32 size);
		
		/**
		 * Read from the buffer.
		 * @param data Buffer to store read data
		 * @param max_to_read Maximum amount of bytes to read
		 * @return The number of bytes read
		 */
		Uint32 read(Uint8* data,Uint32 max_to_read);
		
		/**
		 * Send the data in the buffer over the socket
		 * @param s THe socket
		 * @param max Maximum bytes to send
		 * @return The number of bytes written
		 */
		Uint32 send(BufferedSocket* s,Uint32 max);
	};

}

#endif
