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
#include "bufferedsocket.h"
#include "circularbuffer.h"

using namespace bt;

namespace net
{

	BufferedSocket::BufferedSocket(int fd) : Socket(fd),rdr(0),wrt(0)
	{
		bytes_in_output_buffer = 0;
		bytes_sent = 0;
		outstanding_bytes = 0;
		outstanding_bytes_transmitted = 0;
	}
	
	BufferedSocket::BufferedSocket(bool tcp) : Socket(tcp),rdr(0),wrt(0)
	{
		bytes_in_output_buffer = 0;
		bytes_sent = 0;
		outstanding_bytes = 0;
		outstanding_bytes_transmitted = 0;
	}


	BufferedSocket::~BufferedSocket()
	{
	}

	Uint32 BufferedSocket::readBuffered(Uint32 max_bytes_to_read)
	{	
		Uint8 tmp[4096];
		Uint32 br = 0;
		bool no_limit = (max_bytes_to_read == 0);
			
		while ((br < max_bytes_to_read || no_limit) && bytesAvailable() > 0)
		{
			Uint32 tr = bytesAvailable();
			if (tr > 4096)
				tr = 4096;
			if (!no_limit && tr + br > max_bytes_to_read)
				tr = max_bytes_to_read - br;
			
			int ret = Socket::recv(tmp,tr);
			if (ret != 0)
			{
				if (rdr)
					rdr->onDataReady(tmp,ret);
				br += ret;
			}
			else
			{
				// connection closed, so just return the number of bytes read
	//			Out() << "BufferedSocket::readBuffered " << br << endl;
				return br;
			}
		}
	//	Out() << "BufferedSocket::readBuffered " << br << endl;
		return br;
	}
	
	Uint32 BufferedSocket::sendOutputBuffer(Uint32 max)
	{
		if (bytes_in_output_buffer == 0)
			return 0;
		
		if (max == 0 || bytes_in_output_buffer <= max)
		{
			// try to send everything
			Uint32 bw = bytes_in_output_buffer;
			Uint32 off = bytes_sent;
			Uint32 ret = Socket::send(output_buffer + off,bw);
			bytes_in_output_buffer -= ret;
			bytes_sent += ret;
			if (bytes_sent == bytes_in_output_buffer)
				bytes_in_output_buffer = bytes_sent = 0;
			return ret;
		}
		else 
		{
			Uint32 bw = max;
			Uint32 off = bytes_sent;
			Uint32 ret = Socket::send(output_buffer + off,bw);
			bytes_in_output_buffer -= ret;
			bytes_sent += ret;
			return ret;
		}
	}
	
	Uint32 BufferedSocket::writeBuffered(Uint32 max)
	{
		// we now know that all previously outstanding_bytes are written
		mutex.lock();
		outstanding_bytes_transmitted += outstanding_bytes;
		outstanding_bytes = 0;
		mutex.unlock();
		
		if (!wrt)
			return 0;
		
		Uint32 bw = 0;
		bool no_limit = max == 0;
		if (bytes_in_output_buffer > 0)
		{
			Uint32 ret = sendOutputBuffer(max);
			if (bytes_in_output_buffer > 0)
			{
				mutex.lock();
				outstanding_bytes += ret;
				mutex.unlock();
				// haven't sent it fully so return
				return ret; 
			}
			else if (!no_limit)
				max -= ret; // decrease limit when there is none
			
			bw += ret;
		}
		
		// run as long as we do not hit the limit and we can send everything
		while ((no_limit || bw < max) && bytes_in_output_buffer == 0)
		{
			// fill output buffer
			bytes_in_output_buffer = wrt->onReadyToWrite(output_buffer,4096);
			bytes_sent = 0;
			if (bytes_in_output_buffer == 0)
				break; // no data provided so just break out of the loop
			
			// try to send 
			bw += sendOutputBuffer(max - bw);
		}
		
		mutex.lock();
		outstanding_bytes += bw;
		mutex.unlock();
		return bw;
	}
	
	Uint32 BufferedSocket::dataWritten() const
	{
		mutex.lock();
		Uint32 ret = outstanding_bytes_transmitted;
		outstanding_bytes_transmitted = 0;
		mutex.unlock();
		return ret;
	}
}
