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
#ifndef BTPACKETWRITER_H
#define BTPACKETWRITER_H

#include <qptrlist.h>
#include "globals.h"

namespace bt
{
	class Peer;
	class Request;
	class Chunk;
	class BitSet;
	class Packet;

	/**
	@author Joris Guisson
	*/
	class PacketWriter
	{
		Peer* peer;
		QPtrList<Packet> packets;
		Uint32 uploaded;
	public:
		PacketWriter(Peer* peer);
		virtual ~PacketWriter();

		/**
		 * Send a choke packet.
		 */
		void sendChoke();
		
		/**
		 * Send an unchoke packet.
		 */
		void sendUnchoke();
		
		/**
		 * Send an interested packet.
		 */
		void sendInterested();
		
		/**
		 * Send a not interested packet.
		 */
		void sendNotInterested();
		
		/**
		 * Send a keep alive packet.
		 */
		//void sendKeepAlive();
		
		/**
		 * Send a request for data.
		 * @param req The Request
		 */
		void sendRequest(const Request & r);
		
		/**
		 * Cancel a request.
		 * @param req The Request
		 */
		void sendCancel(const Request & r);
		
		/**
		 * Send a have packet.
		 * @param index
		 */
		void sendHave(Uint32 index);
		
		/**
		 * Send a chunk of data.
		 * @param index Index of chunk
		 * @param begin Offset into chunk
		 * @param len Length of data
		 * @param ch The Chunk
		 */
		void sendChunk(Uint32 index,Uint32 begin,Uint32 len,const Chunk & ch);
		
		/**
		 * Send a BitSet. The BitSet indicates which chunks we have.
		 * @param bs The BitSet
		 */
		void sendBitSet(const BitSet & bs);

		/**
		 * Try to send the remaining packets in the queue.
		 * @return bytes written
		 */
		Uint32 update();

		/// Get the number of packets which need to be written
		Uint32 getNumPacketsToWrite() const {return packets.count();}

		/**
		 * Called by the upload cap, to tell the PacketWriter it
		 * can send some bytes
		 * @param num_bytes Num bytes allowed
		 * @return The number of bytes uploaded
		 */
		Uint32 uploadUnsentBytes(Uint32 num_bytes);
	private:
		void sendPacket(const Packet & p);
	};

}

#endif
