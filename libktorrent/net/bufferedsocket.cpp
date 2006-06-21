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

	BufferedSocket::BufferedSocket(int fd,Uint32 rbuf_size,Uint32 wbuf_size) : Socket(fd),rbuf(rbuf_size),wbuf(wbuf_size),bytes_sent(0)
	{
	}
	
	BufferedSocket::BufferedSocket(bool tcp,Uint32 rbuf_size,Uint32 wbuf_size) : Socket(tcp),rbuf(rbuf_size),wbuf(wbuf_size),bytes_sent(0)
	{
	}


	BufferedSocket::~BufferedSocket()
	{
	}

	Uint32 BufferedSocket::readBuffered(Uint32 max_bytes_to_read)
	{
		if (rbuf.freeSpace() == 0)
			return 0;
		
		Uint8 tmp[512];
		Uint32 br = 0;
		bool no_limit = (max_bytes_to_read == 0);
			
		while ((br < max_bytes_to_read || no_limit) && bytesAvailable() > 0 && rbuf.freeSpace() > 0)
		{
			Uint32 tr = bytesAvailable();
			if (tr > 512)
				tr = 512;
			if (tr > rbuf.freeSpace())
				tr = rbuf.freeSpace();
			if (!no_limit && tr + br > max_bytes_to_read)
				tr = max_bytes_to_read - br;
			
			int ret = Socket::recv(tmp,tr);
			if (ret != 0)
			{
				rbuf.write(tmp,ret);
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
	
	Uint32 BufferedSocket::read(Uint8* data,Uint32 max_to_read)
	{
		return rbuf.read(data,max_to_read);
	}

	Uint32 BufferedSocket::bytesBufferedAvailable() const
	{
		return rbuf.capacity() - rbuf.freeSpace();
	}
	
	Uint32 BufferedSocket::writeBuffered(Uint32 max)
	{
		Uint32 brw = bytesReadyToWrite();
		if (!brw)
			return 0;
		
		return wbuf.send(this,max);
	}
	
	Uint32 BufferedSocket::write(const Uint8* data,Uint32 nb)
	{
		return wbuf.write(data,nb);
	}
	
	Uint32 BufferedSocket::bytesReadyToWrite() const
	{
		return wbuf.capacity() - wbuf.freeSpace();
	}
	
	Uint32 BufferedSocket::getBytesSent()
	{
		mutex.lock();
		Uint32 ret = bytes_sent;
		bytes_sent = 0;
		mutex.unlock();
		return ret;
	}
		
	void BufferedSocket::addBytesSent(Uint32 bs)
	{
		mutex.lock();
		bytes_sent += bs;
		mutex.unlock();
	}
}
