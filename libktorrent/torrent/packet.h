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
#ifndef BTPACKET_H
#define BTPACKET_H

#include "globals.h"

class QString;

namespace bt
{
	class BitSet;
	class Request;
	class Chunk;

	/**
	@author Joris Guisson
	*/
	class Packet
	{
		Uint8 hdr[17];
		Uint32 hdr_length;
		Uint8* data;
		Uint32 data_length;
		Uint32 written;
		Chunk* chunk;
	public:
		Packet(Uint8 type);
		Packet(Uint16 port);
		Packet(Uint32 chunk);
		Packet(const BitSet & bs);
		Packet(const Request & req,bool cancel);
		Packet(Uint32 index,Uint32 begin,Uint32 len,Chunk* ch);
		virtual ~Packet();

		Uint8 getType() const {return hdr[4];}
		
		bool isOK() const;
		
		const Uint8* getHeader() const {return hdr;}
		Uint32 getHeaderLength() const {return hdr_length;}
		
		const Uint8* getData() const {return data;}
		Uint32 getDataLength() const {return data_length;}

		void dataWritten(Uint32 bytes) {written += bytes;}
		Uint32 getDataWritten() const {return written;}
		
		/// Make a description of the packet for debug purposes
		QString debugString() const;
	};

}

#endif
