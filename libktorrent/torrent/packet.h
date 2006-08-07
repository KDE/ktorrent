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
#ifndef BTPACKET_H
#define BTPACKET_H

#include "globals.h"

class QString;

namespace bt
{
	class BitSet;
	class Request;
	class Chunk;
	class Peer;

	/**
	 * @author Joris Guisson
	 * 
	 * Packet off data, which gets sent to a Peer
	*/
	class Packet
	{
		Uint8* data;
		Uint32 size;
		Uint32 written;
	public:
		Packet(Uint8 type);
		Packet(Uint16 port);
		Packet(Uint32 chunk,Uint8 type);
		Packet(const BitSet & bs);
		Packet(const Request & req,Uint8 type);
		Packet(Uint32 index,Uint32 begin,Uint32 len,Chunk* ch);
		virtual ~Packet();

		Uint8 getType() const {return data ? data[4] : 0;}
		
		bool isOK() const;
		
		const Uint8* getData() const {return data;}
		Uint32 getDataLength() const {return size;}

		Uint32 isSent() const {return written == size;}
		
		/// Make a description of the packet for debug purposes
		//QString debugString() const;
		
		/**
		 * Put the packet in an output buffer.
		 * @param buf The buffer
		 * @param max_to_put Maximum bytes to put
		 * @param piece Set to true if this is a piece 
		 * @return The number of bytes put in the buffer
		 */
		Uint32 putInOutputBuffer(Uint8* buf,Uint32 max_to_put,bool & piece);
	};

}

#endif
