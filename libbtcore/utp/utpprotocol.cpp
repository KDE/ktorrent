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
		
		const bt::Uint8* bitset = (const bt::Uint8*)sack + 2;
		int byte = (bit - 2) / 8;
		int bit_off = (bit - 2) % 8;
		return bitset[byte] & (0x01 << bit_off);
	}
	
	
	void Ack(utp::SelectiveAck* sack, bt::Uint16 bit)
	{
		// check bounds
		if (bit < 2 || bit > 8*sack->length + 1)
			return;
		
		bt::Uint8* bitset = (bt::Uint8*)sack + 2;
		int byte = (bit - 2) / 8;
		int bit_off = (bit - 2) % 8;
		bitset[byte] |= (0x01 << bit_off);
	}

}