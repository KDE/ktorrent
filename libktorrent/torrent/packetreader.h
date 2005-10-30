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
#ifndef BTPACKETREADER_H
#define BTPACKETREADER_H

#include "globals.h"

namespace KNetwork
{
	class KBufferedSocket;
}

namespace bt
{
	class SpeedEstimater;

	/**
	@author Joris Guisson
	*/
	class PacketReader
	{
		KNetwork::KBufferedSocket* sock;
		SpeedEstimater* speed;
		Uint8* read_buf;
		Uint32 packet_length,read_buf_ptr,serial;
		bool error;
	public:
		PacketReader(KNetwork::KBufferedSocket* sock,SpeedEstimater* speed);
		virtual ~PacketReader();

		bool readPacket();
	
		Uint32 getPacketLength() const {return packet_length;}
		
		const Uint8* getData() const {return read_buf;}
		
		bool ok() const {return !error;}
		
		bool moreData() const;
	private:
		bool newPacket();
	};

}

#endif
