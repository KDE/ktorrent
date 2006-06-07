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


#include <util/log.h>
#include <util/functions.h>
#include "packetreader.h"
#include "speedestimater.h"
#include "peer.h"
#include "downloadcap.h"


namespace bt
{

	PacketReader::PacketReader(Peer* peer,SpeedEstimater* speed) 
		: peer(peer),speed(speed),error(false)
	{
		read_buf_ptr = packet_length = 0;
		read_buf = new Uint8[MAX_PIECE_LEN + 13];
		wait = false;
		current_packet_no_limit = false;
	}


	PacketReader::~PacketReader()
	{
		delete [] read_buf;
		DownloadCap::instance().killed(this);
	}
	
	void PacketReader::proceed(Uint32 bytes)
	{
		if (bytes == 0)
			readPacket(0);
		else if (wait)
			readPacket(bytes);
	}
	
	void PacketReader::update()
	{
		if (wait || peer->bytesAvailable() == 0)
			return;
		
	
		if (read_buf_ptr == 0)
			newPacket();
		else if (current_packet_no_limit)
			readPacket(0);
	}
	
	void PacketReader::newPacket()
	{
		Uint32 available = peer->bytesAvailable();
		read_buf_ptr = 0;
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
		
		if (packet_length > MAX_PIECE_LEN + 13)
		{
			Out() << " packet_length to large " << packet_length << endl;
			Out() << " " << len[0] << " " << len[1] << " "
					<< len[2] << " " << len[3] << endl;
			error = true;
			return;
		}
		
		if (packet_length > 17)
		{
			if (DownloadCap::instance().allow(this,packet_length))
			{
				current_packet_no_limit = true;
				readPacket(0);
			}
			else
			{
				wait = true;
				current_packet_no_limit = false;
			}
		}
		else
		{
			current_packet_no_limit = true;
			readPacket(0);
		}
	}

	void PacketReader::readPacket(Uint32 mb)
	{
		Uint32 available = peer->bytesAvailable();
		if (available > mb && mb != 0)
			available = mb;
	
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
			if (read_buf[0] == PIECE)
			{
				speed->onRead(to_read);
			}
			read_buf_ptr = 0;
			wait = false;
			current_packet_no_limit = false;
			peer->packetReady(read_buf,packet_length);
			update();
		}
	}
	
	bool PacketReader::moreData() const
	{
		return peer->bytesAvailable() > 0;
	}

}
