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

#include <qobject.h>
#include <qvaluelist.h>
#include <qptrlist.h>
#include <util/ptrmap.h>
#include "globals.h"
#include "peerid.h"
#include <util/bitset.h>

namespace mse
{
	class StreamSocket;
}

namespace bt
{
	class Peer;
	class ChunkManager;
	class Torrent;
	class Authenticate;
	class ChunkCounter;

	struct PotentialPeer
	{
		PeerID id;
		QString ip;
		Uint16 port;
	};

	/**
	 * @author Joris Guisson
	 * @brief Manages all the Peers
	 * 
	 * This class manages all Peer objects.
	 * It can also open connections to other peers.
	 */
	class PeerManager : public QObject
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
		
		Uint32 getNumConnectedPeers() const {return peer_list.count();}
		Uint32 getNumPending() const {return num_pending;}
		
		static void setMaxConnections(Uint32 max);
		static Uint32 getMaxConnections() {return max_connections;}

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

		/// Get a BitSet of all available chunks
		const BitSet & getAvailableChunksBitSet() const {return available_chunks;}
		
		/// Get the chunk counter.
		ChunkCounter & getChunkCounter() {return *cnt;};
	
		/// Are we connected to a Peer given it's PeerID ?
		bool connectedTo(const PeerID & peer_id);	
		
	private:
		void peerAuthenticated(Authenticate* auth,bool ok);
		void updateAvailableChunks();

	private slots:
		void onHave(Peer* p,Uint32 index);
		void onBitSetRecieved(const BitSet & bs);
		void onRerunChoker();
		
		
	signals:
		void newPeer(Peer* p);
		void peerKilled(Peer* p);
		
	private:
		PtrMap<Uint32,Peer> peer_map;
		QPtrList<Peer> peer_list,killed;
		QPtrList<Authenticate> pending;
		Uint32 num_seeders,num_leechers,num_pending;
		QValueList<PotentialPeer> potential_peers;
		Torrent & tor;
		bool started;
		BitSet available_chunks;
		ChunkCounter* cnt;
		
		static Uint32 max_connections;
	};

}

#endif
