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
#ifndef BTPEERMANAGER_H
#define BTPEERMANAGER_H


#include <QList>
#include <QSet>
#include <util/ptrmap.h>
#include <peer/peer.h>
#include <peer/peerid.h>
#include <util/bitset.h>
#include <interfaces/peersource.h>
#include <btcore_export.h>

namespace mse
{
	class StreamSocket;
}


namespace KNetwork
{
	class KResolverResults;
}


namespace bt
{
	class PeerConnector;
	class Piece;
	class Torrent;
	class Authenticate;
	class ChunkCounter;
	class PieceDownloader;

	using KNetwork::KResolverResults;


	
	const Uint32 MAX_SIMULTANIOUS_AUTHS = 20;
	
	/// Base class for handling pieces
	class BTCORE_EXPORT PieceHandler
	{
	public:
		virtual ~PieceHandler() {}
		
		virtual void pieceReceived(const Piece & p) = 0;
	};

	/**
	 * @author Joris Guisson
	 * @brief Manages all the Peers
	 * 
	 * This class manages all Peer objects.
	 * It can also open connections to other peers.
	 */
	class BTCORE_EXPORT PeerManager : public QObject
	{
		Q_OBJECT
	public:
		/**
		 * Constructor.
		 * @param tor The Torrent
		 */
		PeerManager(Torrent & tor);
		virtual ~PeerManager();
		

		/**
		 * Check for new connections, update down and upload speed of each Peer.
		 * Initiate new connections. 
		 */
		void update();
		
		/**
		 * Pause the peer connections
		 */
		void pause();
		
		/**
		 * Unpause the peer connections 
		 */
		void unpause();
		
		/**
		 * Remove dead peers.
		 * @return The number of dead ones removed
		 */
		Uint32 clearDeadPeers();

		/**
		 * Get the i'th Peer.
		 * @param index 
		 * @return Peer or 0 if out of range
		 */
		Peer* getPeer(Uint32 index) {return peer_list.at(index);}

		/**
		 * Find a Peer based on it's ID
		 * @param peer_id The ID
		 * @return A Peer or 0, if nothing could be found
		 */
		Peer* findPeer(Uint32 peer_id);
		
		/**
		 * Find a Peer based on it's PieceDownloader
		 * @param pd The PieceDownloader
		 * @return The matching Peer or 0 if none can be found
		 */
		Peer* findPeer(PieceDownloader* pd);
		
		void setWantedChunks(const BitSet & bs);

		/**
		 * Try to connect to some peers
		 */
		void connectToPeers();
		
		/**
		 * Close all Peer connections.
		 */
		void closeAllConnections();
		
		/**
		 * Start listening to incoming requests.
		 */
		void start();
		
		/**
		 * Stop listening to incoming requests.
		 */
		void stop();

		/**
		 * Kill all peers who have been choked longer then @a older_then time.
		 * @param older_then Time in milliseconds
		 */
		void killChokedPeers(Uint32 older_then);
		
		/**
		 * Kill all peers who appear to be stale
		 */
		void killStalePeers();
		
		Uint32 getNumConnectedPeers() const {return peer_list.count();}
		Uint32 getNumPending() const {return num_pending;}
		
		static void setMaxConnections(Uint32 max);
		static Uint32 getMaxConnections() {return max_connections;}
		
		static void setMaxTotalConnections(Uint32 max);
		static Uint32 getMaxTotalConnections() {return max_total_connections;}
		
		static Uint32 getTotalConnections() {return total_connections;}
		
		/// Is the peer manager started
		bool isStarted() const {return started;}

		/// Get the Torrent
		Torrent & getTorrent() {return tor;}

		/**
		 * A new connection is ready for this PeerManager.
		 * @param sock The socket
		 * @param peer_id The Peer's ID
		 * @param support What extensions the peer supports
		 */
		void newConnection(mse::StreamSocket* sock,const PeerID & peer_id,Uint32 support);

		/**
		 * Add a potential peer
		 * @param pp The PotentialPeer
		 */
		void addPotentialPeer(const PotentialPeer & pp);
		
		/**
		 * Kills all connections to seeders. 
		 * This is used when torrent download gets finished 
		 * and we should drop all connections to seeders
		 */
		void killSeeders();
		
		/**
		 * Kills all peers that are not interested for a long time.
		 * This should be used when torrent is seeding ONLY.
		 */
		void killUninterested();

		/// Get a BitSet of all available chunks
		const BitSet & getAvailableChunksBitSet() const {return available_chunks;}
		
		/// Get the chunk counter.
		ChunkCounter & getChunkCounter() {return *cnt;};
	
		/// Are we connected to a Peer given it's PeerID ?
		bool connectedTo(const PeerID & peer_id);	
		
		/**
		 * A peer has authenticated.
		 * @param auth The Authenticate object
		 * @param pcon The PeerConnector
		 * @param ok Whether or not the attempt was succesfull
		 */
		void peerAuthenticated(Authenticate* auth,PeerConnector* pcon,bool ok);
		
		/**
		 * Save the IP's and port numbers of all peers.
		 */
		void savePeerList(const QString & file);
		
		/**
		 * Load the peer list again and add them to the potential peers
		 */
		void loadPeerList(const QString & file);
		
		typedef QList<Peer*>::const_iterator CItr;
		
		CItr beginPeerList() const {return peer_list.begin();}
		CItr endPeerList() const {return peer_list.end();}
		
		/// Is PEX eanbled
		bool isPexEnabled() const {return pex_on;}
		
		/// Enable or disable PEX
		void setPexEnabled(bool on);
		
		/// Set the group IDs of each peer
		void setGroupIDs(Uint32 up,Uint32 down);
		
		/// Have message received by a peer
		void have(Peer* p,Uint32 index);
		
		/// Bitset received by a peer
		void bitSetReceived(Peer* p,const BitSet & bs);
		
		/// Rerun the choker
		void rerunChoker();
		
		/// A PEX message was received
		void pex(const QByteArray & arr);
		
		/// A port packet was received
		void portPacketReceived(const QString & ip,Uint16 port);
		
		/// A Piece was received
		void pieceReceived(const Piece & p);
		
		/// Set the piece handler
		void setPieceHandler(PieceHandler* ph);
	public slots:
		/**
		 * A PeerSource, has new potential peers.
		 * @param ps The PeerSource
		 */
		void peerSourceReady(PeerSource* ps);
		
	private:
		void updateAvailableChunks();
		bool killBadPeer();
		void createPeer(mse::StreamSocket* sock,const PeerID & peer_id,Uint32 support,bool local);
		bool connectedTo(const QString & ip,Uint16 port) const;

	private slots:
		void onResolverResults(KNetwork::KResolverResults res);
		
	signals:
		void newPeer(Peer* p);
		void peerKilled(Peer* p);
		void stopped();
		
	private:
		PtrMap<Uint32,Peer> peer_map;
		QList<Peer*> peer_list;
		QList<Peer*> killed;
		Torrent & tor;
		bool started;
		BitSet available_chunks, wanted_chunks;
		ChunkCounter* cnt;
		Uint32 num_pending;
		bool pex_on;
		bool wanted_changed;
		PieceHandler* piece_handler;
		bool paused;
		QSet<PeerConnector*> connectors;
		
		static Uint32 max_connections;
		static Uint32 max_total_connections;
		static Uint32 total_connections;
		
		std::multimap<QString,PotentialPeer> potential_peers;
		
		typedef std::multimap<QString,PotentialPeer>::iterator PPItr;
	};

}

#endif
