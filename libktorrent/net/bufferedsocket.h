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
#ifndef NETBUFFEREDSOCKET_H
#define NETBUFFEREDSOCKET_H

#include <net/socket.h>
#include "circularbuffer.h"

namespace net
{

	/**
	 * @author Joris Guisson <joris.guisson@gmail.com>
	 * 
	 * Extends the Socket class with a circular read buffer.
	 */
	class BufferedSocket : public Socket
	{
		CircularBuffer rbuf;
		CircularBuffer wbuf;
		Uint32 bytes_sent;
		QMutex mutex;
	public:
		BufferedSocket(int fd,Uint32 rbuf_size,Uint32 wbuf_size);
		BufferedSocket(bool tcp,Uint32 rbuf_size,Uint32 wbuf_size);
		virtual ~BufferedSocket();

		
		/**
		 * Reads data from the socket to the buffer.
		 * @param max_bytes_to_read Maximum number of bytes to read (0 is no limit)
		 * @return The number of bytes read
		 */
		Uint32 readBuffered(Uint32 max_bytes_to_read);
		
		/**
		 * Read from the buffer
		 * @param data Buffer to store read data
		 * @param max_to_read Maximum amount of bytes to read
		 * @return The number of bytes read
		 */
		Uint32 read(Uint8* data,Uint32 max_to_read);
		
		/// Get the number of bytes available in the read buffer
		Uint32 bytesBufferedAvailable() const;
		
		/**
		 * Writes data from the buffer to the socket.
		 * @param max The maximum number of bytes to send over the socket (0 = no limit)
		 * @return The number of bytes written
		 */
		Uint32 writeBuffered(Uint32 max);
		
		/**
		 * Write data to the write buffer.
		 * @param data The data
		 * @param nb The number of bytes
		 * @return The number of bytes placed into the write buffer
		 */
		Uint32 write(const Uint8* data,Uint32 nb);
		
		/**
		 * @return The number of bytes ready to write.
		 */
		Uint32 bytesReadyToWrite() const;
		
		/// Get the number of bytes sent
		/// Also resets this variable to zero
		Uint32 getBytesSent();
		
		/// Add bytes sent.
		void addBytesSent(Uint32 bs);
		
		/// Get the amountof free space in the write buffer
		Uint32 freeSpaceInWriteBuffer() const {return wbuf.freeSpace();}
	};

}

#endif
