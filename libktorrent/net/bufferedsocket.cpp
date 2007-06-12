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
#define OUTPUT_BUFFER_SIZE 16393

	BufferedSocket::BufferedSocket(int fd) : Socket(fd),rdr(0),wrt(0),up_gid(0),down_gid(0)
	{
		bytes_in_output_buffer = 0;
		bytes_sent = 0;
		down_speed = new Speed();
		up_speed = new Speed();
		output_buffer = new Uint8[OUTPUT_BUFFER_SIZE];
		poll_index = -1;
	}
	
	BufferedSocket::BufferedSocket(bool tcp) : Socket(tcp),rdr(0),wrt(0),up_gid(0),down_gid(0)
	{
		bytes_in_output_buffer = 0;
		bytes_sent = 0;
		down_speed = new Speed();
		up_speed = new Speed();
		output_buffer = new Uint8[OUTPUT_BUFFER_SIZE];
		poll_index = -1;
	}


	BufferedSocket::~BufferedSocket()
	{
		delete [] output_buffer;
		delete up_speed;
		delete down_speed;
	}
	
	void BufferedSocket::setGroupID(Uint32 gid,bool upload)
	{
		if (upload)
			up_gid = gid;
		else
			down_gid = gid;
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
	
	static Uint8 input_buffer[OUTPUT_BUFFER_SIZE];

	Uint32 BufferedSocket::readBuffered(Uint32 max_bytes_to_read,bt::TimeStamp now)
	{	
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
			if (tr > OUTPUT_BUFFER_SIZE)
				tr = OUTPUT_BUFFER_SIZE;
			if (!no_limit && tr + br > max_bytes_to_read)
				tr = max_bytes_to_read - br;
			
			int ret = Socket::recv(input_buffer,tr);
			if (ret != 0)
			{
				mutex.lock();
				down_speed->onData(ret,now);
				mutex.unlock();
				if (rdr)
					rdr->onDataReady(input_buffer,ret);
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
				bytes_in_output_buffer -= ret;
				bytes_sent += ret;
				if (bytes_sent == bytes_in_output_buffer)
					bytes_in_output_buffer = bytes_sent = 0;
				return ret;
			}
			else
			{
				return 0;
			}
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
				bytes_in_output_buffer -= ret;
				bytes_sent += ret;
				return ret;
			}
			else
			{
				return 0;
			}
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
			bytes_in_output_buffer = wrt->onReadyToWrite(output_buffer,OUTPUT_BUFFER_SIZE);
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
	
	void BufferedSocket::updateSpeeds(bt::TimeStamp now)
	{
		up_speed->update(now);
		down_speed->update(now);
	}
}
