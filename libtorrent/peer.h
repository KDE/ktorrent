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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef BTPEER_H
#define BTPEER_H


#include <qsocket.h>
#include <ksharedptr.h>
#include "bitset.h"
#include "globals.h"
#include <libutil/timer.h>
#include "peerid.h"

namespace bt
{
	class Chunk;
	class SpeedEstimater;
	class Peer;
	class Request;
	class Piece;
	class PacketReader;
	class PacketWriter;
	

	/**
	 * @author Joris Guisson
	 * @brief Manages the connection with a peer
	 * 
	 * This class manages a connection with a peer in the P2P network.
	 * It provides functions for sending packets. Packets it recieves
	 * get relayed to the outside world using a bunch of signals.
	*/
	class Peer : public QObject,public KShared
	{
		Q_OBJECT
	public:
		/**
		 * Constructor, set the socket.
		 * The socket is allready opened.
		 * @param sock The socket
		 * @param num_chunks The number of chunks in the file
		 */
		Peer(QSocket* sock,const PeerID & peer_id,Uint32 num_chunks);		
		virtual ~Peer();

		/// See if the peer has been killed.
		bool isKilled() const {return killed;}

		/// Get the PacketWriter
		PacketWriter & getPacketWriter() {return *pwriter;}
		
		/// Is the Peer choked
		bool isChoked() const {return choked;}

		/// Is the Peer interested
		bool isInterested() const {return interested;}

		/// Are we interested in the Peer
		bool areWeInterested() const {return am_interested;}

		/// Are we choked for the Peer
		bool areWeChoked() const {return am_choked;}

		/// Are we being snubbed by the Peer
		bool isSnubbed() const;

		/// Get the upload rate in bytes per sec
		Uint32 getUploadRate() const;

		/// Get the download rate in bytes per sec
		Uint32 getDownloadRate() const;

		/// Get the Peer's BitSet
		const BitSet & getBitSet() const {return pieces;}

		/// Get the Peer's ID
		const PeerID & getPeerID() const {return peer_id;}

		/// Update the up- and down- speed
		void updateSpeed();

		/**
		 * Send a chunk of data.
		 * @param data The data
		 * @param len The length
		 * @param record This packet contributes to the upload rate
		 */
		void sendData(const Uint8* data,Uint32 len,bool record = false);
		
		/**
		 * See if all previously written data, has been sent.
		 */
		bool readyToSend() const;
		
		
		/**
		 * Close the peers connection.
		 */
		void closeConnection();
	
	private slots:
		void connectionClosed(); 
		void readyRead();
		void error(int err);

	signals:
		/**
		 * The Peer has a Chunk.
		 * @param p The Peer
		 * @param index Index of Chunk
		 */
		void haveChunk(Peer* p,Uint32 index);
		
		/**
		 * The Peer sent a request.
		 * @param req The Request
		 */
		void request(const Request & req);
		
		/**
		 * The Peer sent a cancel.
		 * @param req The Request
		 */
		void canceled(const Request & req);
		
		/**
		 * The Peer sent a piece of a Chunk.
		 * @param p The Piece
		 */
		void piece(const Piece & p);
		
	private:
		void readPacket();
		void handlePacket(Uint32 len);
		

	private:
		QSocket* sock;
		
		bool choked,interested,am_choked,am_interested,killed;
		BitSet pieces;
		PeerID peer_id;
		Timer snub_timer;
	
		SpeedEstimater* speed;
		PacketReader* preader;
		PacketWriter* pwriter;

		friend class PacketWriter;
	};

	typedef KSharedPtr<Peer> PeerPtr;
}

#endif
