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
#include "speed.h"

using namespace bt;

namespace net
{

	BufferedSocket::BufferedSocket(int fd) : Socket(fd),rdr(0),wrt(0)
	{
		bytes_in_output_buffer = 0;
		bytes_sent = 0;
		down_speed = new Speed();
		up_speed = new Speed();
	}
	
	BufferedSocket::BufferedSocket(bool tcp) : Socket(tcp),rdr(0),wrt(0)
	{
		bytes_in_output_buffer = 0;
		bytes_sent = 0;
		down_speed = new Speed();
		up_speed = new Speed();
	}


	BufferedSocket::~BufferedSocket()
	{
		delete up_speed;
		delete down_speed;
	}
	
	float BufferedSocket::getDownloadRate() const
	{
		mutex.lock();
		float ret = down_speed->getRate();
		mutex.unlock();
		return ret;
	}
	
	float BufferedSocket::getUploadRate() const
	{
		mutex.lock();
		float ret = up_speed->getRate();
		mutex.unlock();
		return ret;
	}

	Uint32 BufferedSocket::readBuffered(Uint32 max_bytes_to_read,bt::TimeStamp now)
	{	
		Uint8 tmp[4096];
		Uint32 br = 0;
		bool no_limit = (max_bytes_to_read == 0);
		
		if (bytesAvailable() == 0)
		{
			close();
			return 0;
		}
			
		while ((br < max_bytes_to_read || no_limit)  && bytesAvailable() > 0)
		{
			Uint32 tr = bytesAvailable();
			if (tr > 4096)
				tr = 4096;
			if (!no_limit && tr + br > max_bytes_to_read)
				tr = max_bytes_to_read - br;
			
			int ret = Socket::recv(tmp,tr);
			if (ret != 0)
			{
				mutex.lock();
				down_speed->onData(ret,now);
				mutex.unlock();
				if (rdr)
					rdr->onDataReady(tmp,ret);
				br += ret;
			}
			else
			{
				// connection closed, so just return the number of bytes read
				return br;
			}
		}
		return br;
	}
	
	Uint32 BufferedSocket::sendOutputBuffer(Uint32 max,bt::TimeStamp now)
	{
		if (bytes_in_output_buffer == 0)
			return 0;
		
		if (max == 0 || bytes_in_output_buffer <= max)
		{
			// try to send everything
			Uint32 bw = bytes_in_output_buffer;
			Uint32 off = bytes_sent;
			Uint32 ret = Socket::send(output_buffer + off,bw);
			if (ret > 0)
			{
				mutex.lock();
				up_speed->onData(ret,now);
				mutex.unlock();
			}
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
			if (ret > 0)
			{
				mutex.lock();
				up_speed->onData(ret,now);
				mutex.unlock();
			}
			bytes_in_output_buffer -= ret;
			bytes_sent += ret;
			return ret;
		}
	}
	
	Uint32 BufferedSocket::writeBuffered(Uint32 max,bt::TimeStamp now)
	{
		if (!wrt)
			return 0;
		
		Uint32 bw = 0;
		bool no_limit = max == 0;
		if (bytes_in_output_buffer > 0)
		{
			Uint32 ret = sendOutputBuffer(max,now);
			if (bytes_in_output_buffer > 0)
			{
				// haven't sent it fully so return
				return ret; 
			}
			
			bw += ret;
		}
		
		// run as long as we do not hit the limit and we can send everything
		while ((no_limit || bw < max) && bytes_in_output_buffer == 0)
		{
			// fill output buffer
			bytes_in_output_buffer = wrt->onReadyToWrite(output_buffer,4096);
			bytes_sent = 0;
			if (bytes_in_output_buffer > 0)
			{
				// try to send 
				bw += sendOutputBuffer(max - bw,now);
			}
			else
			{
				// no bytes available in output buffer so break
				break;
			}
		}
		
		return bw;
	}
	
	void BufferedSocket::updateUpSpeed()
	{
		up_speed->update();
	}
	
	void BufferedSocket::updateDownSpeed()
	{
		down_speed->update();
	}
	
	void BufferedSocket::updateSpeeds()
	{
		up_speed->update();
		down_speed->update();
	}
}
