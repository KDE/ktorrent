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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
 
//#define LOG_PACKET
#ifdef LOG_PACKET
#include <sys/types.h>
#include <unistd.h>
#endif

#include <util/log.h>
#include <util/file.h>
#include <util/functions.h>
#include "packetreader.h"
#include "peer.h"


namespace bt
{
#ifdef LOG_PACKET
	static void LogPacket(const Uint8* data,Uint32 size,Uint32 len)
	{
		QString file = QString("/tmp/kt-packetreader-%1.log").arg(getpid());
		File fptr;
		if (!fptr.open(file,"a"))
			return;
		
		
		QString tmp = QString("PACKET len = %1, type = %2\nDATA: \n").arg(len).arg(data[0]);
		
		fptr.write(tmp.ascii(),tmp.length());
		
		Uint32 j = 0;
		if (size <= 40)
		{
			for (Uint32 i = 0;i < size;i++)
			{
				tmp = QString("0x%1 ").arg(data[i],0,16);
				fptr.write(tmp.ascii(),tmp.length());
				j++;
				if (j > 10)
				{
					fptr.write("\n",1);
					j = 0;
				}
			}
		}
		else
		{
			for (Uint32 i = 0;i < 20;i++)
			{
				tmp = QString("0x%1 ").arg(data[i],0,16);
				fptr.write(tmp.ascii(),tmp.length());
				j++;
				if (j > 10)
				{
					fptr.write("\n",1);
					j = 0;
				}
			}
			tmp = QString("\n ... \n");
			fptr.write(tmp.ascii(),tmp.length());
			for (Uint32 i = size - 20;i < size;i++)
			{
				tmp = QString("0x%1 ").arg(data[i],0,16);
				fptr.write(tmp.ascii(),tmp.length());
				j++;
				if (j > 10)
				{
					fptr.write("\n",1);
					j = 0;
				}
			}
		}
		fptr.write("\n",1);
	}
#endif

	IncomingPacket::IncomingPacket(Uint32 size) : data(0),size(size),read(0)
	{
		data = new Uint8[size];
	}
	
	IncomingPacket::~IncomingPacket()
	{
		delete [] data;
	}
	
	PacketReader::PacketReader(Peer* peer) 
		: peer(peer),error(false)
	{
		packet_queue.setAutoDelete(true);
		len_received = -1;
	}


	PacketReader::~PacketReader()
	{
	}
	
	
	void PacketReader::update()
	{
		if (error)
			return;
		
		mutex.lock();
		// pass packets to peer
		while (packet_queue.count() > 0)
		{
			IncomingPacket* pck = packet_queue.first();
			if (pck->read == pck->size)
			{
				// full packet is read pass it to peer
				peer->packetReady(pck->data,pck->size);
				packet_queue.removeFirst();
			}
			else
			{
				// packet is not yet full, break out of loop
				break;
			}
		}
		mutex.unlock();
	}
	
	Uint32 PacketReader::newPacket(Uint8* buf,Uint32 size)
	{
		Uint32 packet_length = 0;
		Uint32 am_of_len_read = 0;
		if (len_received > 0)
		{
			if (size < 4 - len_received)
			{
				memcpy(len + len_received,buf,size);
				len_received += size;
				return size;
			}
			else
			{
				memcpy(len + len_received,buf,4 - len_received);
				am_of_len_read = 4 - len_received;
				len_received = 0;
				packet_length = ReadUint32(len,0);
				
			}
		}
		else if (size < 4)
		{
			memcpy(len,buf,size);
			len_received = size;
			return size;
		}
		else
		{
			packet_length = ReadUint32(buf,0);
			am_of_len_read = 4;
		}
		
		if (packet_length == 0)
			return am_of_len_read;
		
		if (packet_length > MAX_PIECE_LEN + 13)
		{
			Out(SYS_CON|LOG_DEBUG) << " packet_length too large " << packet_length << endl;

			error = true;
			return size;
		}
		
		IncomingPacket* pck = new IncomingPacket(packet_length);
		packet_queue.append(pck);
		return am_of_len_read + readPacket(buf + am_of_len_read,size - am_of_len_read);
	}

	Uint32 PacketReader::readPacket(Uint8* buf,Uint32 size)
	{
		if (!size)
			return 0;
		
		IncomingPacket* pck = packet_queue.last();
		if (pck->read + size >= pck->size)
		{
			// we can read the full packet
			Uint32 tr = pck->size - pck->read;
			memcpy(pck->data + pck->read,buf,tr);
			pck->read += tr;
			return tr;
		}
		else
		{
			// we can do a partial read
			Uint32 tr = size;
			memcpy(pck->data + pck->read,buf,tr);
			pck->read += tr;
			return tr;
		}
	}
	

	void PacketReader::onDataReady(Uint8* buf,Uint32 size)
	{
		if (error)
			return;
		
		mutex.lock();
		if (packet_queue.count() == 0)
		{
			Uint32 ret = 0;
			while (ret < size && !error)
			{
				ret += newPacket(buf + ret,size - ret);
			}
		}
		else
		{
			Uint32 ret = 0;
			IncomingPacket* pck = packet_queue.last();
			if (pck->read == pck->size) // last packet in queue is fully read
				ret = newPacket(buf,size);
			else
			 	ret = readPacket(buf,size);
			
			while (ret < size  && !error)
			{
				ret += newPacket(buf + ret,size - ret);
			}
		}
		mutex.unlock();
	}
}
