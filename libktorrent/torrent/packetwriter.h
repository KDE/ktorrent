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
#include "cap.h"

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
	class PacketWriter : public Cappable
	{
		Peer* peer;
		QPtrList<Packet> packets;
		Uint32 uploaded;
		Uint32 time_of_last_transmit;
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
		 * Sends an unchoke message but doesn't update the am_choked field so KT still thinks
		 * it is choked (and will not upload to it), this is to punish snubbers.
		 */
		void sendEvilUnchoke();
		
		/**
		 * Send an interested packet.
		 */
		void sendInterested();
		
		/**
		 * Send a not interested packet.
		 */
		void sendNotInterested();
		
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
		 * Send a reject for a request
		 * @param req The Request
		 */
		void sendReject(const Request & r);
		
		/**
		 * Send a have packet.
		 * @param index
		 */
		void sendHave(Uint32 index);
		
		/**
		 * Send an allowed fast packet
		 * @param index
		 */
		void sendAllowedFast(Uint32 index);
		
		/**
		 * Send a chunk of data.
		 * @param index Index of chunk
		 * @param begin Offset into chunk
		 * @param len Length of data
		 * @param ch The Chunk
		 */
		void sendChunk(Uint32 index,Uint32 begin,Uint32 len,Chunk * ch);
		
		/**
		 * Send a BitSet. The BitSet indicates which chunks we have.
		 * @param bs The BitSet
		 */
		void sendBitSet(const BitSet & bs);
		
		/**
		 * Send a port message
		 * @param port The port
		 */
		void sendPort(Uint16 port);
		
		/// Send a have all message
		void sendHaveAll();
		
		/// Send a have none message
		void sendHaveNone();
		
		/**
		 * Send a suggest piece packet
		 * @param index Index of the chunk
		 */
		void sendSuggestPiece(Uint32 index);

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
		 * @param bytes Num bytes to send (0 == all)
		 */
		void uploadUnsentBytes(Uint32 bytes);
		
		void proceed(Uint32 bytes) {uploadUnsentBytes(bytes);}
	private:
		bool sendPacket(Packet & p,Uint32 max);
		void queuePacket(Packet* p,bool ask);
		void sendSmallPackets();
	};

}

#endif
