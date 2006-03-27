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
#include <util/log.h>
#include <util/error.h>
#include "peermanager.h"
#include "peer.h"
#include "bnode.h"
#include "globals.h"
#include "server.h"
#include "authenticate.h"
#include "torrent.h"
#include "uploader.h"
#include "downloader.h"
#include <util/functions.h>
#include <qhostaddress.h>
#include <qsocket.h> 
#include <klocale.h>
#include "ipblocklist.h"
#include "chunkcounter.h"

namespace bt
{
	Uint32 PeerManager::max_connections = 0;

	PeerManager::PeerManager(Torrent & tor)
		: tor(tor),available_chunks(tor.getNumChunks())
	{
		num_seeders = num_leechers = num_pending = 0;
		killed.setAutoDelete(true);
		started = false;
		Globals::instance().getServer().addPeerManager(this);
		cnt = new ChunkCounter(tor.getNumChunks());
	}


	PeerManager::~PeerManager()
	{
		delete cnt;
		Globals::instance().getServer().removePeerManager(this);
		pending.setAutoDelete(true);
		pending.clear();
		peer_list.setAutoDelete(true);
		peer_list.clear();
	}

	void PeerManager::update()
	{
		if (!started || Globals::instance().inCriticalOperationMode())
			return;

		// update the speed of each peer,
		// and get ridd of some killed peers
		QPtrList<Peer>::iterator i = peer_list.begin();
		while (i != peer_list.end())
		{
			Peer* p = *i;
			if (p->isKilled())
			{
				cnt->decBitSet(p->getBitSet());
				updateAvailableChunks();
				i = peer_list.erase(i);
				killed.append(p);
				peer_map.erase(p->getID());
				peerKilled(p);
			}
			else
			{
				p->update();
				i++;
			}
		}

		// check all pending connections, wether they authenticated
		// properly
		QPtrList<Authenticate>::iterator j = pending.begin();
		while (j != pending.end())
		{
			Authenticate* a = *j;
			if (a->isFinished())
			{
				j = pending.erase(j);
				peerAuthenticated(a,a->isSuccesfull());
				delete a;
			}
			else
			{
				j++;
			}
		}

		// connect to some new peers
		connectToPeers();
	}

	void PeerManager::killChokedPeers(Uint32 older_then)
	{
		Out() << "Getting rid of peers which have been choked for a long time" << endl;
		Uint32 now = bt::GetCurrentTime();
		QPtrList<Peer>::iterator i = peer_list.begin();
		Uint32 num_killed = 0;
		while (i != peer_list.end() && num_killed < 20)
		{
			Peer* p = *i;
			if (p->isChoked() && (now - p->getChokeTime()) > older_then)
			{
				p->kill();
				num_killed++;
			}

			i++;
		}
	}
	
	void PeerManager::setMaxConnections(Uint32 max)
	{
		max_connections = max;
	}
	
	void PeerManager::addPotentialPeer(const PotentialPeer & pp)
	{
		potential_peers.append(pp);
	}

	void PeerManager::killSeeders()
	{
		QPtrList<Peer>::iterator i = peer_list.begin();
		while (i != peer_list.end())
		{
			Peer* p = *i;
 			if ( p->isSeeder() )
 				p->kill();
			i++;
		}
	}

	void PeerManager::onHave(Peer*,Uint32 index)
	{
		available_chunks.set(index,true);
		cnt->inc(index);
	}

	void PeerManager::onBitSetRecieved(const BitSet & bs)
	{
		for (Uint32 i = 0;i < bs.getNumBits();i++)
		{
			if (bs.get(i))
			{
				available_chunks.set(i,true);
				cnt->inc(i);
			}
		}
	}
	
#ifdef USE_KNETWORK_SOCKET_CLASSES
	void PeerManager::newConnection(KNetwork::KBufferedSocket* sock,
									const PeerID & peer_id,bool dht_supported)
#else
	void PeerManager::newConnection(QSocket* sock,
									const PeerID & peer_id,bool dht_supported)
#endif
	{
		Uint32 total = peer_list.count() + pending.count();
		if (!started || (max_connections > 0 && total >= max_connections))
		{
			sock->deleteLater();
			return;
		}

		Peer* peer = new Peer(sock,peer_id,tor.getNumChunks(),dht_supported,0);
		connect(peer,SIGNAL(haveChunk(Peer*, Uint32 )),this,SLOT(onHave(Peer*, Uint32 )));
		connect(peer,SIGNAL(bitSetRecieved(const BitSet& )),
				this,SLOT(onBitSetRecieved(const BitSet& )));
		connect(peer,SIGNAL(rerunChoker()),this,SLOT(onRerunChoker()));
		
		peer_list.append(peer);
		peer_map.insert(peer->getID(),peer);
		newPeer(peer);
	}
	
	void PeerManager::peerAuthenticated(Authenticate* auth,bool ok)
	{
		pending.remove(auth);
		num_pending--;
		if (!ok)
			return;
		
		if (connectedTo(auth->getPeerID()))
			return;
			
		Peer* peer = new Peer(
				auth->takeSocket(),auth->getPeerID(),tor.getNumChunks(),auth->supportsDHT(),0);
		connect(peer,SIGNAL(haveChunk(Peer*, Uint32 )),this,SLOT(onHave(Peer*, Uint32 )));
		connect(peer,SIGNAL(bitSetRecieved(const BitSet& )),
				this,SLOT(onBitSetRecieved(const BitSet& )));
		connect(peer,SIGNAL(rerunChoker()),this,SLOT(onRerunChoker()));
		
		peer_list.append(peer);
		peer_map.insert(peer->getID(),peer);
			
		//	Out() << "New peer connected !" << endl;
		newPeer(peer);
	}
		
	bool PeerManager::connectedTo(const PeerID & peer_id)
	{
		if (!started)
			return false;
		
		for (Uint32 j = 0;j < peer_list.count();j++)
		{
			Peer* p = peer_list.at(j);
			if (p->getPeerID() == peer_id)
			{
				return true;
			}
		}
		return false;
	}
	
	void PeerManager::connectToPeers()
	{
		if (peer_list.count() + pending.count() >= max_connections && max_connections > 0)
			return;
		
		Uint32 num = 0;
		if (max_connections > 0)
		{
			Uint32 available = max_connections - (peer_list.count() + pending.count());
			num = available >= potential_peers.count() ? 
					potential_peers.count() : available;
		}
		else
		{
			num = potential_peers.count();
		}

		if (pending.count() > 50)
			return;
		
		if (num > 0)
		{
			Out() << "Connecting to " << num << " peers (" 
					<< potential_peers.count() << ")" << endl;
		}
		
		for (Uint32 i = 0;i < num;i++)
		{
			if (pending.count() > 50)
				return;
			
			PotentialPeer pp = potential_peers.front();
			potential_peers.pop_front();
			
			if (connectedTo(pp.id))
				continue;

			IPBlocklist& ipfilter = IPBlocklist::instance();
			//Out() << "Dodo " << pp.ip << endl;
			if (ipfilter.isBlocked(pp.ip))
				continue;
			
			Authenticate* auth = new Authenticate(pp.ip,pp.port,
					tor.getInfoHash(),tor.getPeerID(),*this);
			pending.append(auth);
			num_pending++;
		}
	}
	

	
	Uint32 PeerManager::clearDeadPeers()
	{
		Uint32 num = killed.count();
		killed.clear();
		return num;
	}
	
	void PeerManager::closeAllConnections()
	{
		killed.clear();

		peer_map.clear();
		peer_list.setAutoDelete(true);
		peer_list.clear();
		peer_list.setAutoDelete(false);
		
		pending.setAutoDelete(true);
		pending.clear();
		pending.setAutoDelete(false);
	}
	
	void PeerManager::start()
	{
		started = true;
	}
		
	
	void PeerManager::stop()
	{
		started = false;
	}

	Peer* PeerManager::findPeer(Uint32 peer_id)
	{
		return peer_map.find(peer_id);
	}
	
	void PeerManager::onRerunChoker()
	{
		// append a 0 ptr to killed
		// so that the next update in TorrentControl
		// will be forced to do the choking
		killed.append(0);
	}
	
	void PeerManager::updateAvailableChunks()
	{
		for (Uint32 i = 0;i < available_chunks.getNumBits();i++)
		{
			available_chunks.set(i,cnt->get(i) > 0);
		}
	}
	
	
}
#include "peermanager.moc"
