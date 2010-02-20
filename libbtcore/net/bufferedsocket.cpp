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
#include "bufferedsocket.h"
#include <util/log.h>
#include "speed.h"

using namespace bt;

namespace net
{
#define OUTPUT_BUFFER_SIZE 16393

	BufferedSocket::BufferedSocket(SocketDevice* sock) : rdr(0),wrt(0),up_gid(0),down_gid(0),sock(sock)
	{
		bytes_in_output_buffer = 0;
		bytes_sent = 0;
		down_speed = new Speed();
		up_speed = new Speed();
		output_buffer = new Uint8[OUTPUT_BUFFER_SIZE];
	}

	BufferedSocket::BufferedSocket(int fd,int ip_version) : rdr(0),wrt(0),up_gid(0),down_gid(0)
	{
		sock = new Socket(fd,ip_version);
		bytes_in_output_buffer = 0;
		bytes_sent = 0;
		down_speed = new Speed();
		up_speed = new Speed();
		output_buffer = new Uint8[OUTPUT_BUFFER_SIZE];
	}
	
	BufferedSocket::BufferedSocket(bool tcp,int ip_version) : rdr(0),wrt(0),up_gid(0),down_gid(0)
	{
		sock = new Socket(tcp,ip_version);
		bytes_in_output_buffer = 0;
		bytes_sent = 0;
		down_speed = new Speed();
		up_speed = new Speed();
		output_buffer = new Uint8[OUTPUT_BUFFER_SIZE];
	}


	BufferedSocket::~BufferedSocket()
	{
		delete [] output_buffer;
		delete up_speed;
		delete down_speed;
		delete sock;
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
		Uint32 ba = sock->bytesAvailable();
		if (ba == 0)
		{
			sock->close();
			return 0;
		}
			
		while ((br < max_bytes_to_read || no_limit)  && ba > 0)
		{
			Uint32 tr = ba;
			if (tr > OUTPUT_BUFFER_SIZE)
				tr = OUTPUT_BUFFER_SIZE;
			if (!no_limit && tr + br > max_bytes_to_read)
				tr = max_bytes_to_read - br;
			
			int ret = sock->recv(input_buffer,tr);
			if (ret > 0)
			{
				mutex.lock();
				down_speed->onData(ret,now);
				mutex.unlock();
				if (rdr)
					rdr->onDataReady(input_buffer,ret);
				br += ret;
			}
			else if (ret < 0)
			{
				return br;
			}
			else
			{
				sock->close();
				return br;
			}
		}
		return br;
	}
	
	Uint32 BufferedSocket::sendOutputBuffer(Uint32 max,bt::TimeStamp now)
	{
		if (bytes_in_output_buffer == 0)
			return 0;
		
		Uint32 bw = 0;
		Uint32 off = 0;
		if (max == 0 || bytes_in_output_buffer <= max)
		{
			// try to send everything
			bw = bytes_in_output_buffer;
			off = bytes_sent;
		}
		else 
		{
			bw = max;
			off = bytes_sent;
		}
		
		Uint32 ret = sock->send(output_buffer + off,bw);
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
		mutex.lock();
		up_speed->update(now);
		down_speed->update(now);
		mutex.unlock();
	}
}
