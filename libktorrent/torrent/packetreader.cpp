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
#include "speedestimater.h"
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

	
	PacketReader::PacketReader(Peer* peer,SpeedEstimater* speed) 
		: peer(peer),speed(speed),error(false)
	{
		packet_length = 0;
		data_read = 0;
		type = 0;
	}


	PacketReader::~PacketReader()
	{
	}
	
	
	void PacketReader::update()
	{
		if (error)
			return;
		
		if (packet_length == 0)
		{
			if (peer->bytesAvailable() > 0)
				newPacket();
		}
		else 
		{
			// only update if there is new data
			if (peer->bytesAvailable() > data_read)
				readPacket();
		}
	}
	
	void PacketReader::newPacket()
	{
		data_read = 0;
		type = 0;
		type_read = false;
		Uint32 available = peer->bytesAvailable();
		if (available < 4)
			return;
		
		Uint8 len[4];
		if (peer->readData(len,4) != 4)
		{
			error = true;
			return;
		}
			
		packet_length = ReadUint32(len,0);
		
		// a keep alive message
		if (packet_length == 0)
			return;
		
		if (peer->bytesAvailable() >= 1)
		{
			// read the type
			peer->readData(&type,1);
			type_read = true;
		}
		else
		{
			return;
		}
		
		
		if (packet_length > MAX_PIECE_LEN + 13)
		{
			Out() << " packet_length too large " << packet_length << endl;

			Out() << " " << len[0] << " " << len[1] << " "
					<< len[2] << " " << len[3] << endl;
			packet_length = 0;
			error = true;
			return;
		}
		
		if (packet_length == 1)
		{
			peer->packetReady(&type,1);
			packet_length = 0;
		}
		else
		{
			readPacket();
		}
	}

	void PacketReader::readPacket()
	{
		static Uint8 rbuffer[MAX_PIECE_LEN + 13];
		
		
		Uint32 ba = peer->bytesAvailable();
		if (!type_read)
		{
			if (ba >= 1)
			{
				peer->readData(&type,1);
				type_read = true;
				ba -= 1;
			}
			else
			{
				return;
			}
		}
		
		if (ba < packet_length - 1) // packet length - 1 , we have allready read the first byte
		{
			if (ba > data_read && type == PIECE)
			{
				// update download speed
				speed->onRead(ba - data_read);
				data_read = ba;
			}
			return;
		}
		
		
		Uint32 ret = peer->readData(rbuffer+1,packet_length - 1);
		rbuffer[0] = type;
		if (type == PIECE)
		{
			speed->onRead(packet_length - data_read);
		}
		
		peer->packetReady(rbuffer,packet_length);
		packet_length = 0;
#ifdef LOG_PACKET
		LogPacket(rbuffer,packet_length);
#endif
	}
	
	bool PacketReader::moreData() const
	{
		return packet_length == 0 ? peer->bytesAvailable() > 0 : peer->bytesAvailable() > data_read;
	}

}
