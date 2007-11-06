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
#include <util/file.h>
#include <util/error.h>
#include <net/address.h>
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
#include <qdatetime.h>

using namespace kt;

namespace bt
{
	Uint32 PeerManager::max_connections = 0;
	Uint32 PeerManager::max_total_connections = 0;
	Uint32 PeerManager::total_connections = 0;

	PeerManager::PeerManager(Torrent & tor)
		: tor(tor),available_chunks(tor.getNumChunks())
	{
		killed.setAutoDelete(true);
		started = false;
		
		cnt = new ChunkCounter(tor.getNumChunks());
		num_pending = 0;
		pex_on = !tor.isPrivate();
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
		if (!started)
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
				p->update(this);
				i++;
			}
		}
		
		// connect to some new peers
		connectToPeers();
	}

	void PeerManager::killChokedPeers(Uint32 older_then)
	{
		Out() << "Getting rid of peers which have been choked for a long time" << endl;
		TimeStamp now = bt::GetCurrentTime();
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
		Uint32 sys_max = bt::MaxOpenFiles() - 50; // leave about 50 free for regular files
		max_total_connections = max;
		if (max == 0 || max_total_connections > sys_max)
			max_total_connections = sys_max;
	}
	
	void PeerManager::addPotentialPeer(const PotentialPeer & pp)
	{
		if (potential_peers.size() > 150)
			return;
		
		// avoid duplicates in the potential_peers map
		std::pair<PPItr,PPItr> r = potential_peers.equal_range(pp.ip);
		for (PPItr i = r.first;i != r.second;i++)
		{
			if (i->second.port == pp.port) // port and IP are the same so return
				return;
		}
		
		potential_peers.insert(std::make_pair(pp.ip,pp));
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
	
	void PeerManager::killUninterested()
	{
		QPtrList<Peer>::iterator i = peer_list.begin();
		while (i != peer_list.end())
		{
			Peer* p = *i;
			if ( !p->isInterested() && (p->getConnectTime().secsTo(QTime::currentTime()) > 30) )
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
			// get rid of bad peer and replace it by another one
			if (!killBadPeer())
			{
				// we failed to find a bad peer, so just delete this one
				delete sock;
				return;
			}
		}

		createPeer(sock,peer_id,support,false);
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
				if (auth->isLocal())
					st->setLocal(true);
				
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
			
		createPeer(auth->takeSocket(),auth->getPeerID(),auth->supportedExtensions(),auth->isLocal());
	}
	
	void PeerManager::createPeer(mse::StreamSocket* sock,const PeerID & peer_id,Uint32 support,bool local)
	{
		Peer* peer = new Peer(sock,peer_id,tor.getNumChunks(),tor.getChunkSize(),support,local);
		
		connect(peer,SIGNAL(haveChunk(Peer*, Uint32 )),this,SLOT(onHave(Peer*, Uint32 )));
		connect(peer,SIGNAL(bitSetRecieved(const BitSet& )),
				this,SLOT(onBitSetRecieved(const BitSet& )));
		connect(peer,SIGNAL(rerunChoker()),this,SLOT(onRerunChoker()));
		connect(peer,SIGNAL(pex( const QByteArray& )),this,SLOT(pex( const QByteArray& )));
		
		peer_list.append(peer);
		peer_map.insert(peer->getID(),peer);
		total_connections++;
		newPeer(peer);
		peer->setPexEnabled(pex_on);
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
	
	bool PeerManager::connectedTo(const QString & ip,Uint16 port) const
	{
		PtrMap<Uint32,Peer>::const_iterator i = peer_map.begin();
		while (i != peer_map.end())
		{
			const Peer* p = i->second;
			if (p->getPort() == port && p->getStats().ip_address == ip)
				return true;
			i++;
		}
		return false;
	}
	
	void PeerManager::connectToPeers()
	{
		if (potential_peers.size() == 0)
			return;
		
		if (peer_list.count() + num_pending >= max_connections && max_connections > 0)
			return;
		
		if (total_connections >= max_total_connections && max_total_connections > 0)
			return;
		
		if (num_pending > MAX_SIMULTANIOUS_AUTHS)
			return;
		
		if (!mse::StreamSocket::canInitiateNewConnection())
			return; // to many sockets in SYN_SENT state
		
		Uint32 num = 0;
		if (max_connections > 0)
		{
			Uint32 available = max_connections - (peer_list.count() + num_pending);
			num = available >= potential_peers.size() ? 
					potential_peers.size() : available;
		}
		else
		{
			num = potential_peers.size();
		}
		
		if (num + total_connections >= max_total_connections && max_total_connections > 0)
			num = max_total_connections - total_connections;
		
		for (Uint32 i = 0;i < num;i++)
		{
			if (num_pending > MAX_SIMULTANIOUS_AUTHS)
				return;
			
			PPItr itr = potential_peers.begin();
			
			IPBlocklist& ipfilter = IPBlocklist::instance();
			
			if (!ipfilter.isBlocked(itr->first) && !connectedTo(itr->first,itr->second.port))
			{
			//	Out() << "EncryptedAuthenticate : " << pp.ip << ":" << pp.port << endl;
				Authenticate* auth = 0;
				const PotentialPeer & pp = itr->second;
				
				if (Globals::instance().getServer().isEncryptionEnabled())
					auth = new mse::EncryptedAuthenticate(pp.ip,pp.port,tor.getInfoHash(),tor.getPeerID(),this);
				else
					auth = new Authenticate(pp.ip,pp.port,tor.getInfoHash(),tor.getPeerID(),this);
				
				if (pp.local)
					auth->setLocal(true);
				
				connect(this,SIGNAL(stopped()),auth,SLOT(onPeerManagerDestroyed()));
				
				AuthenticationMonitor::instance().add(auth);
				num_pending++;
				total_connections++;
			}
			potential_peers.erase(itr);
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
	
	// pick a random magic number
	const Uint32 PEER_LIST_HDR_MAGIC = 0xEF12AB34;
	
	struct PeerListHeader
	{
		Uint32 magic;
		Uint32 num_peers;
		Uint32 ip_version; // 4 or 6, 6 is for future purposes only (when we support IPv6)
	};
	
	struct PeerListEntry
	{
		Uint32 ip;
		Uint16 port;
	};
	
	void PeerManager::savePeerList(const QString & file)
	{
		bt::File fptr;
		if (!fptr.open(file,"wb"))
			return;
		
		try
		{
			PeerListHeader hdr;
			hdr.magic = PEER_LIST_HDR_MAGIC;
			// we will save both the active and potential peers
			hdr.num_peers = peer_list.count() + potential_peers.size();
			hdr.ip_version = 4;
			
			fptr.write(&hdr,sizeof(PeerListHeader));
			
			Out(SYS_GEN|LOG_DEBUG) << "Saving list of peers to " << file << endl;
			// first the active peers
			for (QPtrList<Peer>::iterator itr = peer_list.begin(); itr != peer_list.end();itr++)
			{
				Peer* p = *itr;
				PeerListEntry e;
				net::Address addr = p->getAddress();
				e.ip = addr.ip();
				e.port = addr.port();
				fptr.write(&e,sizeof(PeerListEntry));
			}
			
			// now the potential_peers
			PPItr i = potential_peers.begin();
			while (i != potential_peers.end())
			{
				net::Address addr(i->first,i->second.port);
				PeerListEntry e;
				e.ip = addr.ip();
				e.port = addr.port();
				fptr.write(&e,sizeof(PeerListEntry));
				i++;
			}
		}
		catch (bt::Error & err)
		{
			Out(SYS_GEN|LOG_DEBUG) << "Error happened during saving of peer list : " << err.toString() << endl;
		}
	}
	
	void PeerManager::loadPeerList(const QString & file)
	{
		bt::File fptr;
		if (!fptr.open(file,"rb"))
			return;
		
		try
		{
			PeerListHeader hdr;
			fptr.read(&hdr,sizeof(PeerListHeader));
			if (hdr.magic != PEER_LIST_HDR_MAGIC || hdr.ip_version != 4)
				throw Error("Peer list file corrupted");
			
			Out(SYS_GEN|LOG_DEBUG) << "Loading list of peers from " << file << " (num_peers =  " << hdr.num_peers << ")" << endl;
			
			for (Uint32 i = 0;i < hdr.num_peers && !fptr.eof();i++)
			{
				PeerListEntry e;
				fptr.read(&e,sizeof(PeerListEntry));
				PotentialPeer pp;
				
				// convert IP address to string 
				pp.ip = QString("%1.%2.%3.%4")
						.arg((e.ip & 0xFF000000) >> 24)
						.arg((e.ip & 0x00FF0000) >> 16)
						.arg((e.ip & 0x0000FF00) >> 8)
						.arg( e.ip & 0x000000FF);
				pp.port = e.port;
				addPotentialPeer(pp);
			}
			
		}
		catch (bt::Error & err)
		{
			Out(SYS_GEN|LOG_DEBUG) << "Error happened during saving of peer list : " << err.toString() << endl;
		}
	}
	
	void PeerManager::start()
	{
		started = true;
		Globals::instance().getServer().addPeerManager(this);
	}
		
	
	void PeerManager::stop()
	{
		cnt->reset();
		available_chunks.clear();
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
	
	void PeerManager::peerSourceReady(kt::PeerSource* ps)
	{
		PotentialPeer pp;
		while (ps->takePotentialPeer(pp))
			addPotentialPeer(pp);
	}
	
	bool PeerManager::killBadPeer()
	{
		for (PtrMap<Uint32,Peer>::iterator i = peer_map.begin();i != peer_map.end();i++)
		{
			Peer* p = i->second;
			if (p->getStats().aca_score <= -5.0 && p->getStats().aca_score > -50.0)
			{
				Out(SYS_GEN|LOG_DEBUG) << "Killing bad peer, to make room for other peers" << endl;
				p->kill();
				return true;
			}
		}
		return false;
	}
	
	void PeerManager::pex(const QByteArray & arr)
	{
		if (!pex_on)
			return;
		
		Out(SYS_CON|LOG_NOTICE) << "PEX: found " << (arr.size() / 6) << " peers"  << endl;
		for (Uint32 i = 0;i+6 <= arr.size();i+=6)
		{
			Uint8 tmp[6];
			memcpy(tmp,arr.data() + i,6);
			PotentialPeer pp;
			pp.port = ReadUint16(tmp,4);
			Uint32 ip = ReadUint32(tmp,0);
			pp.ip = QString("%1.%2.%3.%4")
						.arg((ip & 0xFF000000) >> 24)
						.arg((ip & 0x00FF0000) >> 16)
						.arg((ip & 0x0000FF00) >> 8)
						.arg( ip & 0x000000FF);
			pp.local = false;
			
			addPotentialPeer(pp);
		}
	}
	
	
	void PeerManager::setPexEnabled(bool on)
	{
		if (on && tor.isPrivate())
			return;
		
		if (pex_on == on)
			return;
		
		QPtrList<Peer>::iterator i = peer_list.begin();
		while (i != peer_list.end())
		{
			Peer* p = *i;
			if (!p->isKilled())
				p->setPexEnabled(on);
			i++;
		}
		pex_on = on;
	}
	
	void PeerManager::setGroupIDs(Uint32 up,Uint32 down)
	{
		for (PtrMap<Uint32,Peer>::iterator i = peer_map.begin();i != peer_map.end();i++)
		{
			Peer* p = i->second;
			p->setGroupIDs(up,down);
		}
	}
}

#include "peermanager.moc"
