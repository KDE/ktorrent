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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/


#include <util/log.h>
#include <util/functions.h>
#include "packetreader.h"
#include "speedestimater.h"
#include "peer.h"


namespace bt
{
	static Uint32 dodo = 0;


	PacketReader::PacketReader(Peer* peer,SpeedEstimater* speed) 
		: peer(peer),speed(speed),error(false)
	{
		read_buf_ptr = packet_length = 0;
		read_buf = new Uint8[MAX_PIECE_LEN + 13];
		serial = dodo;
		dodo++;
	}


	PacketReader::~PacketReader()
	{
		delete [] read_buf;
	}
	
	bool PacketReader::newPacket()
	{
		Uint32 available = peer->bytesAvailable();
		read_buf_ptr = 0;
		if (available < 4)
			return false;
		
		Uint8 len[4];
		if (peer->readData(len,4) != 4)
		{
			error = true;
			return false;
		}
			
		packet_length = ReadUint32(len,0);
	//	Out() << serial << " packet_length = " << packet_length << endl;
		if (packet_length > MAX_PIECE_LEN + 13)
		{
			Out() << serial << " packet_length to large " << packet_length << endl;
			Out() << " " << len[0] << " " << len[1] << " "
					<< len[2] << " " << len[3] << endl;
			error = true;
			return false;
		}
		
		// a keep alive message
		if (packet_length == 0)
			return true;
			
		available = peer->bytesAvailable();
		// see if the entire packet is available
		if (available < packet_length)
		{
			// not enough for the entire packet so store in read bufer
			peer->readData(read_buf,available);
			read_buf_ptr += available;
			if (read_buf[0] == PIECE)
			{
		//		Out() << serial << " available = " << available << endl;
				speed->onRead(available);
			}
		}
		else
		{
			peer->readData(read_buf,packet_length);
			if (read_buf[0] == PIECE)
			{
				speed->onRead(packet_length);
			//	Out() << serial << " Packet finished " << packet_length << endl;
			}
			read_buf_ptr = 0;
			return true;
		}
		return false;
	}

	bool PacketReader::readPacket()
	{
		// packet_length > 0 indicates that
		// we're busy reading a big package
		if (read_buf_ptr == 0)
			return newPacket();
		
		Uint32 available = peer->bytesAvailable();
		//	Out() << serial << " available = " << available << endl;
		//	Out() << serial << " accum = " << (read_buf_ptr + available) << " " << packet_length << endl;
		if (read_buf_ptr + available < packet_length)
		{
			peer->readData(read_buf + read_buf_ptr,available);
			read_buf_ptr += available;
			if (read_buf[0] == PIECE)
			{
				speed->onRead(available);
			}
		}
		else
		{
			Uint32 to_read = packet_length - read_buf_ptr;
			peer->readData(read_buf + read_buf_ptr,to_read);
			//	Out() << serial << " Packet finished " << packet_length << " " << 
			//			(packet_length - read_buf_ptr) << endl;
			if (read_buf[0] == PIECE)
			{
				//	Out() << serial << " Packet finished " << packet_length << " " << to_read << endl;
				speed->onRead(to_read);
			}
			read_buf_ptr = 0;
			return true;
		}
		
		return false;
	}
	
	bool PacketReader::moreData() const
	{
		return peer->bytesAvailable() > 0;
	}

}
