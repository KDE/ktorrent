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

#include <qmutex.h>
#include <net/socket.h>

namespace net
{
	using bt::Uint8;
	using bt::Uint32;
	
	class Speed;
	
	class SocketReader
	{
	public:
		/**
		 * Function which will be called whenever data has been read from the socket.
		 * This data should be dealt with, otherwise it will be discarded.
		 * @param buf The buffer
		 * @param size The size of the buffer
		 */
		virtual void onDataReady(Uint8* buf,Uint32 size) = 0;	
	};
	
	class SocketWriter
	{
	public:		
		/**
		 * The socket is ready to write, the writer is asked to provide the data.
		 * The data will be fully sent, before another request is done.
		 * @param data The data
		 * @param max_to_write The maximum number of bytes to put in the buffer
		 * @param The number of bytes placed in the buffer 
		 */
		virtual Uint32 onReadyToWrite(Uint8* data,Uint32 max_to_write) = 0;	
		
		/// Check if data is ready to write
		virtual bool hasBytesToWrite() const = 0;

	};

	/**
	 * @author Joris Guisson <joris.guisson@gmail.com>
	 * 
	 * Extends the Socket class with
	 */
	class BufferedSocket : public Socket
	{
		mutable QMutex mutex;
		SocketReader* rdr;
		SocketWriter* wrt;
		Uint8 output_buffer[4096];
		Uint32 bytes_in_output_buffer; // bytes in the output buffer
		Uint32 bytes_sent; // bytes written of the output buffer
		Speed* down_speed;
		Speed* up_speed;
	public:
		BufferedSocket(int fd);
		BufferedSocket(bool tcp);
		virtual ~BufferedSocket();

		
		void setReader(SocketReader* r) {rdr = r;}
		void setWriter(SocketWriter* r) {wrt = r;}
		
		/**
		 * Reads data from the socket to the buffer.
		 * @param max_bytes_to_read Maximum number of bytes to read (0 is no limit)
		 * @param now Current time stamp
		 * @return The number of bytes read
		 */
		Uint32 readBuffered(Uint32 max_bytes_to_read,bt::TimeStamp now);
		
		/**
		 * Writes data from the buffer to the socket.
		 * @param max The maximum number of bytes to send over the socket (0 = no limit)
		 * * @param now Current time stamp
		 * @return The number of bytes written
		 */
		Uint32 writeBuffered(Uint32 max,bt::TimeStamp now);
		
		/// See if the socket has something ready to write
		bool bytesReadyToWrite() const {return !wrt ? false : wrt->hasBytesToWrite();}
	
		
		/// Get the current download rate
		float getDownloadRate() const;
		
		/// Get the current download rate
		float getUploadRate() const;
		
		/// Update the speeds of the socket
		void updateUpSpeed();
		
		/// Update the speeds of the socket
		void updateDownSpeed();
		
		/// Update up and down speed
		void updateSpeeds();
		
	private:
		Uint32 sendOutputBuffer(Uint32 max,bt::TimeStamp now);
	};

}

#endif
