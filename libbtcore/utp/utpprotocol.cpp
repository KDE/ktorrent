/***************************************************************************
 *   Copyright (C) 2009 by Joris Guisson                                   *
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
#include "utpprotocol.h"
#include <util/functions.h>

namespace utp
{
	/*
	UTP standard:
	The bitmask has reverse byte order. The first byte represents packets [ack_nr + 2, ack_nr + 2 + 7] in reverse order. The least significant bit in the byte represents ack_nr + 2, the most significant bit in the byte represents ack_nr + 2 + 7. The next byte in the mask represents [ack_nr + 2 + 8, ack_nr + 2 + 15] in reverse order, and so on. The bitmask is not limited to 32 bits but can be of any size.
	
	Here is the layout of a bitmask representing the first 32 packet acks represented in a selective ACK bitfield:
	
	0               8               16
	+---------------+---------------+---------------+---------------+
	| 9 8 ...   3 2 | 17   ...   10 | 25   ...   18 | 33   ...   26 |
	+---------------+---------------+---------------+---------------+
	
	The number in the diagram maps the bit in the bitmask to the offset to add to ack_nr in order to calculate the sequence number that the bit is ACKing.
	*/
	bool Acked(const SelectiveAck* sack,bt::Uint16 bit)
	{
		// check bounds
		if (bit < 2 || bit > 8*sack->length + 1)
			return false;
		
		const bt::Uint8* bitset = sack->bitmask;
		int byte = (bit - 2) / 8;
		int bit_off = (bit - 2) % 8;
		return bitset[byte] & (0x01 << bit_off);
	}
	
	
	void Ack(utp::SelectiveAck* sack, bt::Uint16 bit)
	{
		// check bounds
		if (bit < 2 || bit > 8*sack->length + 1)
			return;
		
		bt::Uint8* bitset = sack->bitmask;
		int byte = (bit - 2) / 8;
		int bit_off = (bit - 2) % 8;
		bitset[byte] |= (0x01 << bit_off);
	}
	
	QString TypeToString(bt::Uint8 type)
	{
		switch (type)
		{
			case ST_DATA: return QString("DATA");
			case ST_FIN: return QString("FIN");
			case ST_STATE: return QString("STATE");
			case ST_RESET: return QString("RESET");
			case ST_SYN: return QString("SYN");
			default: return QString("UNKNOWN");
		}
	}
	
	void Header::read(const bt::Uint8* data)
	{
		type = (data[0] & 0xF0) >> 4;
		version = data[0] & 0x0F;
		extension = data[1];
		connection_id = bt::ReadUint16(data,2);
		timestamp_microseconds = bt::ReadUint32(data,4);
		timestamp_difference_microseconds = bt::ReadUint32(data,8);
		wnd_size = bt::ReadUint32(data,12);
		seq_nr = bt::ReadUint16(data,16);
		ack_nr = bt::ReadUint16(data,18);
	}
	
	void Header::write(bt::Uint8* data)
	{
		data[0] = ((type << 4) & 0xF0) | (version & 0x0F);
		data[1] = extension;
		bt::WriteUint16(data,2,connection_id);
		bt::WriteUint32(data,4,timestamp_microseconds);
		bt::WriteUint32(data,8,timestamp_difference_microseconds);
		bt::WriteUint32(data,12,wnd_size);
		bt::WriteUint16(data,16,seq_nr);
		bt::WriteUint16(data,18,ack_nr);
	}
	
	bt::Uint32 Header::size()
	{
		return 20;
	}

	
	PacketParser::PacketParser(const QByteArray& pkt) 
		: packet((const bt::Uint8*)pkt.data()),size(pkt.size()),sack_found(false),data_off(0),data_size(0)
	{
		hdr.read(packet);
	}

	PacketParser::PacketParser(const bt::Uint8* packet, bt::Uint32 size) 
		: packet(packet),size(size),sack_found(false),data_off(0),data_size(0)
	{
		hdr.read(packet);
	}

	PacketParser::~PacketParser()
	{
	}

	bool PacketParser::parse()
	{
		if (size < Header::size())
			return false;
		
		data_off =  Header::size();
		
		// go over all header extensions to increase the data offset and watch out for selective acks
		int ext_id = hdr.extension;
		while (data_off < size && ext_id != 0)
		{
			const bt::Uint8* ptr = packet + data_off;
			if (ext_id == SELECTIVE_ACK_ID)
			{
				sack_found = true;
				sack.extension = ptr[0];
				sack.length = ptr[1];
				if (data_off + 2 + sack.length > size)
					return false;
				sack.bitmask = (bt::Uint8*)ptr + 2;
			}
			
			data_off += 2 + ptr[1];
			ext_id = ptr[0];
		}
		
		data_size = size - data_off;
		return true;
	}

	const utp::SelectiveAck* PacketParser::selectiveAck() const
	{
		return sack_found ? &sack : 0;
	}

}