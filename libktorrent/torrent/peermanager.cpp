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
#include <mse/streamsocket.h> 
#include <mse/encryptedauthenticate.h>
#include <klocale.h>
#include "ipblocklist.h"
#include "chunkcounter.h"
#include "authenticationmonitor.h"

namespace bt
{
	Uint32 PeerManager::max_connections = 0;
	Uint32 PeerManager::max_total_connections = 0;
	Uint32 PeerManager::total_connections = 0;

	PeerManager::PeerManager(Torrent & tor)
		: tor(tor),available_chunks(tor.getNumChunks())
	{
		num_seeders = num_leechers = num_pending = 0;
		killed.setAutoDelete(true);
		started = false;
		
		cnt = new ChunkCounter(tor.getNumChunks());
	}


	PeerManager::~PeerManager()
	{
		delete cnt;
		Globals::instance().getServer().removePeerManager(this);
		
		if (peer_list.count() <= total_connections)
			total_connections -= peer_list.count();
		else
			total_connections = 0;
		
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
				if (total_connections > 0)
					total_connections--;
				peerKilled(p);
			}
			else
			{
				p->update();
				i++;
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
	
	void PeerManager::setMaxTotalConnections(Uint32 max)
	{
		max_total_connections = max;
	}
	
	void PeerManager::addPotentialPeer(const PotentialPeer & pp)
	{
//		Out() << "Addding " << pp.ip << ":" << pp.port << endl;
		// lets not add to many potential peer, we can only connect to so many peers
		if (potential_peers.count() < 100)
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
	

	void PeerManager::newConnection(mse::StreamSocket* sock,const PeerID & peer_id,Uint32 support)
	{
		Uint32 total = peer_list.count() + num_pending;
		bool local_not_ok = (max_connections > 0 && total >= max_connections);
		bool global_not_ok = (max_total_connections > 0 && total_connections >= max_total_connections);
		
		if (!started || local_not_ok || global_not_ok)
		{
			delete sock;
			return;
		}

		Peer* peer = new Peer(sock,peer_id,tor.getNumChunks(),support);
		connect(peer,SIGNAL(haveChunk(Peer*, Uint32 )),this,SLOT(onHave(Peer*, Uint32 )));
		connect(peer,SIGNAL(bitSetRecieved(const BitSet& )),
				this,SLOT(onBitSetRecieved(const BitSet& )));
		connect(peer,SIGNAL(rerunChoker()),this,SLOT(onRerunChoker()));
		
		peer_list.append(peer);
		peer_map.insert(peer->getID(),peer);
		total_connections++;
		newPeer(peer);
	}
	
	void PeerManager::peerAuthenticated(Authenticate* auth,bool ok)
	{
		if (!started)
			return;
		
		if (total_connections > 0)
			total_connections--;
		
		num_pending--;
		if (!ok)
		{
			mse::EncryptedAuthenticate* a = dynamic_cast<mse::EncryptedAuthenticate*>(auth);
			if (a && Globals::instance().getServer().unencryptedConnectionsAllowed())
			{
				// if possible try unencrypted
				QString ip = a->getIP();
				Uint16 port = a->getPort();
				Authenticate* st = new Authenticate(ip,port,tor.getInfoHash(),tor.getPeerID(),this);
				connect(this,SIGNAL(stopped()),st,SLOT(onPeerManagerDestroyed()));
				AuthenticationMonitor::instance().add(st);
				num_pending++;
				total_connections++;
			}
			return;
		}
		
		if (connectedTo(auth->getPeerID()))
		{
			return;
		}
			
		Uint32 flags = 0;
		if (auth->supportsDHT())
			flags |= bt::DHT_SUPPORT;
		if (auth->supportsFastExtensions())
			flags |= bt::FAST_EXT_SUPPORT;
		
		Peer* peer = new Peer(auth->takeSocket(),auth->getPeerID(),tor.getNumChunks(),flags);
		connect(peer,SIGNAL(haveChunk(Peer*, Uint32 )),this,SLOT(onHave(Peer*, Uint32 )));
		connect(peer,SIGNAL(bitSetRecieved(const BitSet& )),
				this,SLOT(onBitSetRecieved(const BitSet& )));
		connect(peer,SIGNAL(rerunChoker()),this,SLOT(onRerunChoker()));
		
		peer_list.append(peer);
		peer_map.insert(peer->getID(),peer);
		total_connections++;
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
		if (potential_peers.count() == 0)
			return;
		
		if (peer_list.count() + num_pending >= max_connections && max_connections > 0)
			return;
		
		if (total_connections >= max_total_connections && max_total_connections > 0)
			return;
		
		if (num_pending > MAX_SIMULTANIOUS_AUTHS)
			return;
		
		Uint32 num = 0;
		if (max_connections > 0)
		{
			Uint32 available = max_connections - (peer_list.count() + num_pending);
			num = available >= potential_peers.count() ? 
					potential_peers.count() : available;
		}
		else
		{
			num = potential_peers.count();
		}
		
		if (num + total_connections >= max_total_connections && max_total_connections > 0)
			num = max_total_connections - total_connections;
		
		for (Uint32 i = 0;i < num;i++)
		{
			if (num_pending > MAX_SIMULTANIOUS_AUTHS)
				return;
			
			PotentialPeer pp = potential_peers.front();
			potential_peers.pop_front();
			
			if (connectedTo(pp.id))
				continue;

			IPBlocklist& ipfilter = IPBlocklist::instance();
			
			if (ipfilter.isBlocked(pp.ip))
				continue;
			

		//	Out() << "EncryptedAuthenticate : " << pp.ip << ":" << pp.port << endl;
			Authenticate* auth = 0;
			
			if (Globals::instance().getServer().isEncryptionEnabled())
				auth = new mse::EncryptedAuthenticate(pp.ip,pp.port,
			tor.getInfoHash(),tor.getPeerID(),this);
			else
				auth = new Authenticate(pp.ip,pp.port,tor.getInfoHash(),tor.getPeerID(),this);
			connect(this,SIGNAL(stopped()),auth,SLOT(onPeerManagerDestroyed()));
			AuthenticationMonitor::instance().add(auth);
			num_pending++;
			total_connections++;
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
		
		if (peer_list.count() <= total_connections)
			total_connections -= peer_list.count();
		else
			total_connections = 0;

		peer_map.clear();
		peer_list.setAutoDelete(true);
		peer_list.clear();
		peer_list.setAutoDelete(false);
	}
	
	void PeerManager::start()
	{
		started = true;
		Globals::instance().getServer().addPeerManager(this);
	}
		
	
	void PeerManager::stop()
	{
		started = false;
		Globals::instance().getServer().removePeerManager(this);
		stopped();
		num_pending = 0;
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
