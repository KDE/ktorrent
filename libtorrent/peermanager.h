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
#ifndef BTPEERMANAGER_H
#define BTPEERMANAGER_H

#include <qserversocket.h> 
#include <qvaluelist.h>
#include "ptrlist.h"
#include "globals.h"
#include "peerid.h"

namespace bt
{
	class Peer;
	class ChunkManager;
	class BDictNode;
	class BListNode;
	class Torrent;
	class Authenticate;

	

	/**
	 * @author Joris Guisson
	 * @brief Manages all the Peers
	 * 
	 * This class manages all Peer objects, and listens for incoming
	 * connections. It can also open connections to other peers.
	 */
	class PeerManager : public QServerSocket
	{
		Q_OBJECT
		
		struct PotentialPeer
		{
			PeerID id;
			QString ip;
			Uint16 port;
		};
	public:
		/**
		 * Constructor.
		 * @param tor The Torrent
		 * @param port The port to listen on
		 * @throw Error if we can listen on the port
		 */
		PeerManager(Torrent & tor,Uint16 port);
		virtual ~PeerManager();
		
		/**
		 * Update the down and upload speed of each Peer.
		 */
		void updateSpeed();
		
		/**
		 * Remove dead peers.
		 */
		void clearDeadPeers();

		Peer* getPeer(Uint32 index) {return peers.at(index);}
		
		/**
		 * The tracker has been updated, get the list
		 * of peers from the update.
		 * @param dict The dictionary
		 */
		void trackerUpdate(BDictNode* dict);
		
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
		
		Uint32 getNumConnectedPeers() const {return peers.count();}
		Uint32 getNumPending() const {return num_pending;}
		
		static void setMaxConnections(Uint32 max);
	private:
		virtual void newConnection(int socket);
		void readPotentialPeers(BListNode* n);
		bool connectedTo(const PeerID & peer_id);
		
	private slots:
		void fatalError(Peer* p);
		void peerAuthenticated(Authenticate* auth,bool ok);
		
	signals:
		void newPeer(Peer* p);
		void peerKilled(Peer* p);
		
	private:
		PtrList<Peer> peers,killed;
		PtrList<Authenticate> pending,pending_done;
		Uint32 num_seeders,num_leechers,num_pending;
		QValueList<PotentialPeer> potential_peers;
		Torrent & tor;
		bool started;
		
		static Uint32 max_connections;
	};

};

#endif
