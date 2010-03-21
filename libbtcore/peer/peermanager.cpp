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
#include "peermanager.h"
#include <QtAlgorithms>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <k3resolver.h>
#include <util/log.h>
#include <util/file.h>
#include <util/error.h>
#include <net/address.h>
#include <torrent/torrent.h>
#include <util/functions.h>
#include <mse/streamsocket.h> 
#include <mse/encryptedauthenticate.h>
#include <klocale.h>
#include <peer/accessmanager.h>
#include <torrent/globals.h>
#include <torrent/server.h>
#include <dht/dhtbase.h>
#include "packetwriter.h"
#include "chunkcounter.h"
#include "authenticationmonitor.h"
#include "peer.h"
#include "authenticate.h"
#include "peerconnector.h"

using namespace KNetwork;

namespace bt
{
	Uint32 PeerManager::max_connections = 0;
	Uint32 PeerManager::max_total_connections = 0;
	Uint32 PeerManager::total_connections = 0;

	PeerManager::PeerManager(Torrent & tor)
		: tor(tor),available_chunks(tor.getNumChunks()),wanted_chunks(tor.getNumChunks())
	{
		started = false;
		wanted_chunks.setAll(true);
		wanted_changed = false;
		cnt = new ChunkCounter(tor.getNumChunks());
		num_pending = 0;
		pex_on = !tor.isPrivate();
		piece_handler = 0;
		paused = false;
	}


	PeerManager::~PeerManager()
	{
		delete cnt;
		ServerInterface::removePeerManager(this);
		
		if ((Uint32)peer_list.count() <= total_connections)
			total_connections -= peer_list.count();
		else
			total_connections = 0;
		
		qDeleteAll(peer_list.begin(),peer_list.end());
		peer_list.clear();
	}

	void PeerManager::pause()
	{
		if (paused)
			return;
		
		foreach (Peer* p,peer_list)
		{
			p->pause();
		}
		paused = true;
	}

	void PeerManager::unpause()
	{
		if (!paused)
			return;
		
		foreach (Peer* p,peer_list)
		{
			p->unpause();
			if (p->hasWantedChunks(wanted_chunks)) // send interested when it has wanted chunks
				p->getPacketWriter().sendInterested();
		}
		paused = false;
	}

	void PeerManager::update()
	{
		if (!started)
			return;

		// update the speed of each peer,
		// and get ridd of some killed peers
		QList<Peer*>::iterator i = peer_list.begin();
		while (i != peer_list.end())
		{
			Peer* p = *i;
			if (!p->isKilled() && p->isStalled())
			{
				PotentialPeer pp;
				pp.port = p->getPort();
				pp.local = p->getStats().local;
				pp.ip = p->getIPAddresss();
				p->kill();
				addPotentialPeer(pp);
				Out(SYS_CON|LOG_NOTICE) << QString("Killed stalled peer %1").arg(pp.ip) << endl;
			}
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
		if (wanted_changed)
		{
			foreach (Peer* p,peer_list)
			{
				if (p->hasWantedChunks(wanted_chunks))
					p->getPacketWriter().sendInterested();
				else
					p->getPacketWriter().sendNotInterested();
				i++;
			}
			wanted_changed = false;
		}
	}

	void PeerManager::killChokedPeers(Uint32 older_then)
	{
		Out(SYS_CON|LOG_DEBUG) << "Getting rid of peers which have been choked for a long time" << endl;
		TimeStamp now = bt::CurrentTime();
		QList<Peer*>::iterator i = peer_list.begin();
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
#ifndef Q_WS_WIN
		Uint32 sys_max = bt::MaxOpenFiles() - 50; // leave about 50 free for regular files
#else
		Uint32 sys_max = 9999; // there isn't a real limit on windows
#endif
		max_total_connections = max;
		if (max == 0 || max_total_connections > sys_max)
			max_total_connections = sys_max;
	}
	
	void PeerManager::setWantedChunks(const BitSet & bs)
	{
		wanted_chunks = bs;
		wanted_changed = true;
	}
	
	void PeerManager::addPotentialPeer(const PotentialPeer & pp)
	{
		if (potential_peers.size() > 500)
			return;
		
		KIpAddress addr;
		if (addr.setAddress(pp.ip))
		{
			// avoid duplicates in the potential_peers map
			std::pair<PPItr,PPItr> r = potential_peers.equal_range(pp.ip);
			for (PPItr i = r.first;i != r.second;i++)
			{
				if (i->second.port == pp.port) // port and IP are the same so return
					return;
			}
		
			potential_peers.insert(std::make_pair(pp.ip,pp));
		}
		else
		{
			// must be a hostname, so resolve it
			KResolver::resolveAsync(this,SLOT(onResolverResults(KNetwork::KResolverResults )),
					pp.ip,QString::number(pp.port));
		}
	}
	
	void PeerManager::onResolverResults(KResolverResults res)
	{
		if (res.count() == 0)
			return;
		
		net::Address addr = res.front().address().asInet();
		
		PotentialPeer pp;
		pp.ip = addr.ipAddress().toString();
		pp.port = addr.port();
		pp.local = false;
		
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
		QList<Peer*>::iterator i = peer_list.begin();
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
		QList<Peer*>::iterator i = peer_list.begin();
		while (i != peer_list.end())
		{
			Peer* p = *i;
			if ( !p->isInterested() && (p->getConnectTime().secsTo(QTime::currentTime()) > 30) )
				p->kill();
			i++;
		}
	}

	void PeerManager::have(Peer* p,Uint32 index)
	{
		if (wanted_chunks.get(index) && !paused)
			p->getPacketWriter().sendInterested();
		available_chunks.set(index,true);
		cnt->inc(index);
	}

	void PeerManager::bitSetReceived(Peer* p, const BitSet & bs)
	{
		bool interested = false;
		for (Uint32 i = 0;i < bs.getNumBits();i++)
		{
			if (bs.get(i))
			{
				if (wanted_chunks.get(i))
					interested = true;
				available_chunks.set(i,true);
				cnt->inc(i);
			}
		}
		if (interested && !paused)
			p->getPacketWriter().sendInterested();
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
	
	void PeerManager::peerAuthenticated(Authenticate* auth,PeerConnector* pcon,bool ok)
	{
		if (!started)
		{
			connectors.remove(pcon);
			pcon->deleteLater();
			return;
		}
		
		if (total_connections > 0)
			total_connections--;
		
		num_pending--;
		if (ok && !connectedTo(auth->getPeerID()))
			createPeer(auth->takeSocket(),auth->getPeerID(),auth->supportedExtensions(),auth->isLocal());
		
		connectors.remove(pcon);
		pcon->deleteLater();
	}
	
	void PeerManager::createPeer(mse::StreamSocket* sock,const PeerID & peer_id,Uint32 support,bool local)
	{
		Peer* peer = new Peer(sock,peer_id,tor.getNumChunks(),tor.getChunkSize(),support,local,this);
		peer_list.append(peer);
		peer_map.insert(peer->getID(),peer);
		total_connections++;
		newPeer(peer);
		peer->setPexEnabled(pex_on);
		// send extension protocol handshake
		bt::Uint16 port = ServerInterface::getPort();
		peer->sendExtProtHandshake(port,tor.getMetaData().size());
	}
		
	bool PeerManager::connectedTo(const PeerID & peer_id)
	{
		if (!started)
			return false;
		
		for (int j = 0;j < peer_list.count();j++)
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
		if(paused)
			return;
		
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
			
			AccessManager & aman = AccessManager::instance();
			
			if (aman.allowed(itr->first) && !connectedTo(itr->first,itr->second.port))
			{
				const PotentialPeer & pp = itr->second;
				PeerConnector* pcon = new PeerConnector(pp.ip,pp.port,pp.local,this);
				connectors.insert(pcon);
				num_pending++;
				total_connections++;
			}
			potential_peers.erase(itr);
		}
	}
	

	
	Uint32 PeerManager::clearDeadPeers()
	{
		Uint32 num = killed.count();
		qDeleteAll(killed);
		killed.clear();
		return num;
	}
	
	void PeerManager::closeAllConnections()
	{
		qDeleteAll(killed);
		killed.clear();
	
		if ((Uint32)peer_list.count() <= total_connections)
			total_connections -= peer_list.count();
		else
			total_connections = 0;

		peer_map.clear();
		qDeleteAll(peer_list);
		peer_list.clear();
	}
	
	
	void PeerManager::savePeerList(const QString & file)
	{
		// Lets save the entries line based
		QFile fptr(file);
		if (!fptr.open(QIODevice::WriteOnly))
			return;
		
		
		try
		{
			Out(SYS_GEN|LOG_DEBUG) << "Saving list of peers to " << file << endl;
			
			QTextStream out(&fptr);
			// first the active peers
			foreach(Peer* p,peer_list)
			{
				const net::Address & addr = p->getAddress();
				out << addr.ipAddress().toString() << " " << (unsigned short)addr.port() << ::endl;
			}
			
			// now the potential_peers
			PPItr i = potential_peers.begin();
			while (i != potential_peers.end())
			{
				out << i->first << " " <<  i->second.port << ::endl;
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
		QFile fptr(file);
		if (!fptr.open(QIODevice::ReadOnly))
			return;
		
		try
		{
			
			Out(SYS_GEN|LOG_DEBUG) << "Loading list of peers from " << file  << endl;
			
			while (!fptr.atEnd())
			{
				QStringList sl = QString(fptr.readLine()).split(" ");
				if (sl.count() != 2)
					continue;
				
				bool ok = false;
				PotentialPeer pp;
				pp.ip = sl[0];
				pp.port = sl[1].toInt(&ok);
				if (ok)
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
		unpause();
		ServerInterface::addPeerManager(this);
	}
		
	
	void PeerManager::stop()
	{
		cnt->reset();
		available_chunks.clear();
		started = false;
		ServerInterface::removePeerManager(this);
		foreach (PeerConnector* pcon,connectors)
			pcon->deleteLater();
		connectors.clear();
		stopped();
		num_pending = 0;
	}

	Peer* PeerManager::findPeer(Uint32 peer_id)
	{
		return peer_map.find(peer_id);
	}
	
	Peer* PeerManager::findPeer(PieceDownloader* pd)
	{
		foreach (Peer* p,peer_list)
		{
			if ((PieceDownloader*)p->getPeerDownloader() == pd)
				return p;
		}
		return 0;
	}
	
	void PeerManager::rerunChoker()
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
	
	void PeerManager::peerSourceReady(PeerSource* ps)
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
		for (int i = 0;i+6 <= arr.size();i+=6)
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
		
		QList<Peer*>::iterator i = peer_list.begin();
		while (i != peer_list.end())
		{
			Peer* p = *i;
			if (!p->isKilled())
			{
				p->setPexEnabled(on);
				bt::Uint16 port = ServerInterface::getPort();
				p->sendExtProtHandshake(port,tor.getMetaData().size());
			}
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
		
	void PeerManager::portPacketReceived(const QString& ip, Uint16 port)
	{
		if (Globals::instance().getDHT().isRunning() && !tor.isPrivate())
			Globals::instance().getDHT().portReceived(ip,port);
	}
	
	void PeerManager::pieceReceived(const Piece & p)
	{
		if (piece_handler)
			piece_handler->pieceReceived(p);
	}
		
	void PeerManager::setPieceHandler(PieceHandler* ph)
	{
		piece_handler = ph;
	}
	
	void PeerManager::killStalePeers()
	{
		foreach (Peer* p,peer_list)
		{
			if (p->getDownloadRate() == 0 && p->getUploadRate() == 0)
				p->kill();
		}
	}

}

#include "peermanager.moc"
