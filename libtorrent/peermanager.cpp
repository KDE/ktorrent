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
#include "log.h"
#include "peermanager.h"
#include "peer.h"
#include "bnode.h"
#include "error.h"
#include "authenticate.h"
#include "torrent.h"
#include "uploader.h"
#include "downloader.h"

namespace bt
{
	Uint32 PeerManager::max_connections = 0;

	PeerManager::PeerManager(Torrent & tor,Uint16 port)
	: QServerSocket(port,5),tor(tor)
	{
		num_seeders = num_leechers = num_pending = 0;
		killed.setAutoDelete(true);
		pending_done.setAutoDelete(true);
		started = false;
	}


	PeerManager::~PeerManager()
	{
		pending.setAutoDelete(true);
		peers.setAutoDelete(true);
	}
	
	void PeerManager::setMaxConnections(Uint32 max)
	{
		max_connections = max;
	}
	
	void PeerManager::trackerUpdate(BDictNode* dict)
	{
		if (!started)
			return;
		
		BNode* tmp = dict->getData("complete");
		if (tmp && tmp->getType() == BNode::VALUE)
		{
			BValueNode* vn = (BValueNode*)tmp;
			num_seeders = vn->data().toInt();
		}
			
		tmp = dict->getData("incomplete");
		if (tmp && tmp->getType() == BNode::VALUE)
		{
			BValueNode* vn = (BValueNode*)tmp;
			num_leechers = vn->data().toInt();
		}
		
		BListNode* ln = dict->getList("peers");
		if (!ln)
		{
			// no list, it might however be a compact response
			BValueNode* vn = dict->getValue("peers");
			if (!vn)
				throw Error("Parse error");

			QByteArray arr = vn->data().toByteArray();
			for (int i = 0;i < arr.size();i+=6)
			{
				Uint8 buf[6];
				for (int j = 0;j < 6;j++)
					buf[j] = arr[i + j];

				PotentialPeer pp;
				pp.ip = ReadUint32(buf,0);
				pp.port = ReadUint16(buf,4);
				potential_peers.append(pp);
			}
		}
		else
		{
			readPotentialPeers(ln);
		}
	}

	void PeerManager::trackerUpdate(Uint32 seeders,Uint32 leechers,Uint8* ppeers)
	{
		num_seeders = seeders;
		num_leechers = leechers;
		Uint32 n = num_seeders + num_leechers;

		for (Uint32 i = 0;i < n;i++)
		{
			PotentialPeer pp;
			pp.port = ReadUint16(ppeers,6*i + 4);
			pp.ip = ReadUint32(ppeers,6*i);
			potential_peers.append(pp);
		}
	}

	void PeerManager::newConnection(int socket)
	{
		if (!started)
			return;
		
		Uint32 total = peers.count() + pending.count();
		if (max_connections > 0 && total >= max_connections)
			return;
	
		QSocket* sock = new QSocket();
		sock->setSocket(socket);
		Authenticate* auth = new Authenticate(sock,tor.getInfoHash(),tor.getPeerID());
		connect(auth,SIGNAL(finished(Authenticate*, bool )),
				this,SLOT(peerAuthenticated(Authenticate*, bool )));
		pending.append(auth);
		num_pending++;
	}
	
	void PeerManager::peerAuthenticated(Authenticate* auth,bool ok)
	{
		pending.erase(auth);
		num_pending--;
		pending_done.append(auth);
		if (ok)
		{
			if (connectedTo(auth->getPeerID()))
				return;
			
			Peer* peer = new Peer(
					auth->takeSocket(),auth->getPeerID(),tor.getNumChunks());
			
			connect(peer,SIGNAL(fatalError(Peer* )),
					this,SLOT(fatalError(Peer* )));
			
			peers.append(peer);
			
		//	Out() << "New peer connected !" << endl;
			newPeer(peer);
		}
		else
		{
		//	Out() << "Authentication failed !" << endl;
		}
	}
	
	void PeerManager::readPotentialPeers(BListNode* n)
	{
		if (!started)
			return;
		
		potential_peers.clear();
		Out() << "Reading " << n->getNumChildren() << " potential peers"  << endl;
		for (Uint32 i = 0;i < n->getNumChildren();i++)
		{
			BDictNode* dict = dynamic_cast<BDictNode*>(n->getChild(i));
			if (!dict)
				continue;
			
			PotentialPeer pp;
			
			BValueNode* vn = dynamic_cast<BValueNode*>(dict->getData("ip"));
			if (!vn)
				continue;
			
			pp.ip = vn->data().toString();
			
			vn = dynamic_cast<BValueNode*>(dict->getData("port"));
			if (!vn)
				continue;
			
			pp.port = vn->data().toInt();
			
			vn = dynamic_cast<BValueNode*>(dict->getData("peer id"));
			if (!vn)
				continue;
			
			pp.id = PeerID(vn->data().toByteArray().data());
			potential_peers.append(pp);
		}
	}
	
	void PeerManager::fatalError(Peer* p)
	{
	//	Out() << "Peer connection shutdown " << endl;
		peers.erase(p);
		killed.append(p);
		p->closeConnection();
		peerKilled(p);
	}
	
	bool PeerManager::connectedTo(const PeerID & peer_id)
	{
		if (!started)
			return false;
		
		for (Uint32 j = 0;j < peers.count();j++)
		{
			Peer* p = peers.at(j);
			if (p->getPeerID() == peer_id)
			{
				return true;
			}
		}
		return false;
	}
	
	void PeerManager::connectToPeers()
	{
		if (peers.count() + pending.count() >= max_connections && max_connections > 0)
			return;
		
		Uint32 num = 0;
		if (max_connections > 0)
		{
			Uint32 available = max_connections - (peers.count() + pending.count());
			num = available >= potential_peers.count() ? 
					potential_peers.count() : available;
		}
		else
		{
			num = potential_peers.count();
		}
		
		if (num > 0)
		{
			Out() << "Connecting to " << num << " peers (" 
					<< potential_peers.count() << ")" << endl;
		}
		
		for (Uint32 i = 0;i < num;i++)
		{
			PotentialPeer pp = potential_peers.front();
			potential_peers.pop_front();
			
			if (connectedTo(pp.id))
				continue;
			
			Authenticate* auth = new Authenticate(pp.ip,pp.port,tor.getInfoHash(),tor.getPeerID());
			connect(auth,SIGNAL(finished(Authenticate*, bool )),
					this,SLOT(peerAuthenticated(Authenticate*, bool )));
			pending.append(auth);
			num_pending++;
		}
	}
	
	void PeerManager::updateSpeed()
	{
		if (!started)
			return;
		
		for (Uint32 i = 0;i < peers.count();i++)
		{
			Peer* p = peers.at(i);
			p->updateSpeed();
		}
	}
	
	void PeerManager::clearDeadPeers()
	{
		killed.clear();
		pending_done.clear();
	}
	
	void PeerManager::closeAllConnections()
	{
		killed.clear();
		pending_done.clear();
		
		peers.setAutoDelete(true);
		peers.clear();
		peers.setAutoDelete(false);
		
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
}
;
#include "peermanager.moc"
