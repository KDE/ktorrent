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
#ifndef BTPACKETWRITER_H
#define BTPACKETWRITER_H

#include <list>
#include <qmutex.h>
#include <net/bufferedsocket.h>
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
	class PacketWriter : public net::SocketWriter
	{
		Peer* peer;
		std::list<Packet*> control_packets;
		std::list<Packet*> data_packets;
		Packet* curr_packet;
		Uint32 ctrl_packets_sent;
		mutable Uint32 uploaded;
		mutable Uint32 uploaded_non_data;
		mutable QMutex mutex;
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
		 * @return true If we satisfy the request, false otherwise
		 */
		bool sendChunk(Uint32 index,Uint32 begin,Uint32 len,Chunk * ch);
		
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
		
		/// Send the extension protocol handshake
		void sendExtProtHandshake(Uint16 port,bool pex_on = true);
		
		/// Send an extended protocol message
		void sendExtProtMsg(Uint8 id,const QByteArray & data);

		/// Get the number of packets which need to be written
		Uint32 getNumPacketsToWrite() const;
		
		/// Get the number of data packets to write
		Uint32 getNumDataPacketsToWrite() const;
		
		/// Get the number of data bytes uploaded
		Uint32 getUploadedDataBytes() const;
		
		/// Get the number of bytes uploaded
		Uint32 getUploadedNonDataBytes() const;

		/**
		 * Do not send a piece which matches this request.
		 * But only if we are not allready sending the piece.
		 * @param req The request
		 * @param reject Wether we can send a reject instead
		 */
		void doNotSendPiece(const Request & req,bool reject);
		
		/**
		 * Clear all pieces we are not in the progress of sending.
		 * @param reject Send a reject packet
		 */
		void clearPieces(bool reject);
	
	private:
		void queuePacket(Packet* p);
		Packet* selectPacket();
		virtual Uint32 onReadyToWrite(Uint8* data,Uint32 max_to_write);
		virtual bool hasBytesToWrite() const;
	};

}

#endif
