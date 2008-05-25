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
#ifndef BTPEER_H
#define BTPEER_H

#include <qobject.h>
#include <qdatetime.h>
#include <util/timer.h>
#include <interfaces/peerinterface.h>
#include <util/bitset.h>
#include "globals.h"
#include "peerid.h"

namespace net
{
	class Address;
}


namespace mse
{
	class RC4Encryptor;
	class StreamSocket;
}

namespace bt
{
	class Chunk;
	class Peer;
	class Request;
	class Piece;
	class PacketReader;
	class PacketWriter;
	class PeerDownloader;
	class PeerUploader;
	class PeerManager;
	class UTPex;
	

	

	/**
	 * @author Joris Guisson
	 * @brief Manages the connection with a peer
	 * 
	 * This class manages a connection with a peer in the P2P network.
	 * It provides functions for sending packets. Packets it receives
	 * get relayed to the outside world using a bunch of signals.
	*/
	class Peer : public QObject, public kt::PeerInterface
			  //,public Object
	{
		Q_OBJECT
	public:
		/**
		 * Constructor, set the socket.
		 * The socket is already opened.
		 * @param sock The socket
		 * @param peer_id The Peer's BitTorrent ID
		 * @param num_chunks The number of chunks in the file
		 * @param chunk_size Size of each chunk 
		 * @param support Which extensions the peer supports
		 * @param local Wether or not it is a local peer
		 */
		Peer(mse::StreamSocket* sock,
			 const PeerID & peer_id,
			 Uint32 num_chunks,
			 Uint32 chunk_size,
			 Uint32 support,
			 bool local);
		
		virtual ~Peer();

		/// Get the peer's unique ID.
		Uint32 getID() const {return id;}
		
		/// Get the IP address of the Peer.
		QString getIPAddresss() const;
		
		/// Get the port of the Peer
		Uint16 getPort() const;
		
		/// Get the address of the peer
		net::Address getAddress() const;
		
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

		/// Update the up- and down- speed and handle incoming packets
		void update(PeerManager* pman);

		/// Get the PeerDownloader.
		PeerDownloader* getPeerDownloader() {return downloader;}

		/// Get the PeerUploader.
		PeerUploader* getPeerUploader() {return uploader;}
		
		/**
		 * Send a chunk of data.
		 * @param data The data
		 * @param len The length
		 * @param proto Indicates wether the packed is data or a protocol message
		 * @return Number of bytes written
		 */
		Uint32 sendData(const Uint8* data,Uint32 len);
		
		/**
		 * Reads data from the peer.
		 * @param buf The buffer to store the data
		 * @param len The maximum number of bytes to read
		 * @return The number of bytes read
		 */
		Uint32 readData(Uint8* buf,Uint32 len);
		
		/// Get the number of bytes available to read.
		Uint32 bytesAvailable() const;
		
		/**
		 * See if all previously written data, has been sent.
		 */
		bool readyToSend() const;
		
		
		/**
		 * Close the peers connection.
		 */
		void closeConnection();

		/**
		 * Kill the Peer.
		 */
		void kill();

		/**
		 * Get the time when this Peer was choked.
		 */
		TimeStamp getChokeTime() const {return time_choked;}
		
		/**
		 * Get the time when this Peer was unchoked.
		 */
		TimeStamp getUnchokeTime() const {return time_unchoked;}

		/**
		 * See if the peer is a seeder.
		 */
		bool isSeeder() const;

		/// Get the time in milliseconds since the last time a piece was received.
		Uint32 getTimeSinceLastPiece() const;

		/// Get the time the peer connection was established.
		const QTime & getConnectTime() const {return connect_time;}

		/**
		 * Get the percentual amount of data available from peer.
		 */
		float percentAvailable() const;

		/// See if the peer supports DHT
		bool isDHTSupported() const {return stats.dht_support;}
		
		/// Set the ACA score
		void setACAScore(double s);
		
		/// Get the stats of the peer
		virtual const Stats & getStats() const;
		
		/// Choke the peer
		void choke();
		
		/**
		 *  Emit the port packet signal.
		 */
		void emitPortPacket();
		
		/**
		 * Emit the pex signal
		 */
		void emitPex(const QByteArray & data);
		
		/// Disable or enable pex
		void setPexEnabled(bool on);
		
		/**
		 * Set the peer's group IDs for traffic 
		 * @param up_gid The upload gid
		 * @param down_gid The download gid
		 */
		void setGroupIDs(Uint32 up_gid,Uint32 down_gid);
		
	private slots:
		void dataWritten(int bytes);

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

		/**
		 * Recieved a BitSet
		 * @param bs The BitSet
		 */
		void bitSetRecieved(const BitSet & bs);

		/**
		 * Emitted when the peer is unchoked and interested changes value.
		 */
		void rerunChoker();
		
		/**
		 * Got a port packet from this peer.
		 * @param ip The IP
		 * @param port The port
		 */
		void gotPortPacket(const QString & ip,Uint16 port);
		
		/**
		 * A Peer Exchange has been received, the QByteArray contains the data.
		 */
		void pex(const QByteArray & data);
		
	private:
		void packetReady(const Uint8* packet,Uint32 size);
		void handleExtendedPacket(const Uint8* packet,Uint32 size);

	private:
		mse::StreamSocket* sock;
		bool choked;
		bool interested;
		bool am_choked;
		bool am_interested;
		bool killed;
		TimeStamp time_choked;
		TimeStamp time_unchoked;
		Uint32 id;
		BitSet pieces;
		PeerID peer_id;
		Timer snub_timer;
		PacketReader* preader;
		PacketWriter* pwriter;
		PeerDownloader* downloader;
		PeerUploader* uploader;
		mutable kt::PeerInterface::Stats stats;
		QTime connect_time;
		UTPex* ut_pex;
		bool pex_allowed;
		Uint32 utorrent_pex_id;

		friend class PacketWriter;
		friend class PacketReader;
		friend class PeerDownloader;
	};
}

#endif
