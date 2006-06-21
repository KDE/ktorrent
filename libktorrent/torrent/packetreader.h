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
#ifndef BTPACKETREADER_H
#define BTPACKETREADER_H

#include "globals.h"

namespace bt
{
	class SpeedEstimater;
	class Peer;

	/**
	@author Joris Guisson
	*/
	class PacketReader
	{
		Peer* peer;
		SpeedEstimater* speed;
		Uint32 packet_length;
		Uint8 type;
		Uint32 data_read;
		bool error;
		bool type_read;
	public:
		PacketReader(Peer* peer,SpeedEstimater* speed);
		virtual ~PacketReader();

		void update();
		Uint32 getPacketLength() const {return packet_length;}
		bool ok() const {return !error;}
		bool moreData() const;
	private:
		void newPacket();
		void readPacket();
	};

}

#endif
