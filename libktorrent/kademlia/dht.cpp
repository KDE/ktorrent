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
#include <qmap.h>
#include <kresolver.h>
#include <util/log.h>
#include <util/array.h>
#include <util/functions.h>
#include <torrent/bnode.h>
#include <torrent/globals.h>
#include <ksocketaddress.h>
#include "announcetask.h"
#include "dht.h"
#include "node.h"
#include "rpcserver.h"
#include "rpcmsg.h"
#include "kclosestnodessearch.h"
#include "database.h"
#include "taskmanager.h"
#include "nodelookup.h"


using namespace bt;
using namespace KNetwork;

namespace dht
{
	


	DHT::DHT() : node(0),srv(0),db(0),tman(0)
	{
		connect(&update_timer,SIGNAL(timeout()),this,SLOT(update()));
	}


	DHT::~DHT()
	{
		if (running)
			stop();
	}
	
	void DHT::start(const QString & table,const QString & key_file,bt::Uint16 port)
	{
		if (running)
			return;
		
		if (port == 0)
			port = 6881; 
		
		table_file = table;
		this->port = port;
		Out(SYS_DHT|LOG_NOTICE) << "DHT: Starting on port " << port << endl;
		srv = new RPCServer(this,port);
		node = new Node(srv,key_file);
		db = new Database();
		tman = new TaskManager();
		expire_timer.update();
		running = true;
		srv->start();
		node->loadTable(table);
		update_timer.start(1000);
		started();
	}
		
		
	void DHT::stop()
	{
		if (!running)
			return;
		
		update_timer.stop();
		Out(SYS_DHT|LOG_NOTICE) << "DHT: Stopping " << endl;
		srv->stop();
		node->saveTable(table_file);
		running = false;
		stopped();
		delete tman; tman = 0;
		delete db; db = 0;
		delete node; node = 0;
		delete srv; srv = 0;
	}

	void DHT::ping(PingReq* r)
	{
		if (!running)
			return;
		
		// ignore requests we get from ourself
		if (r->getID() == node->getOurID())
			return;
		
		Out(SYS_DHT|LOG_NOTICE) << "DHT: Sending ping response" << endl;
		PingRsp rsp(r->getMTID(),node->getOurID());
		rsp.setOrigin(r->getOrigin());
		srv->sendMsg(&rsp);
		node->recieved(this,r);
	}
	
	
	
	void DHT::findNode(FindNodeReq* r)
	{
		if (!running)
			return;
		
		// ignore requests we get from ourself
		if (r->getID() == node->getOurID())
			return;
		
		Out(SYS_DHT|LOG_DEBUG) << "DHT: got findNode request" << endl;
		node->recieved(this,r);
		// find the K closest nodes and pack them
		KClosestNodesSearch kns(r->getTarget(),K);
		
		node->findKClosestNodes(kns);
		
		Uint32 rs = kns.requiredSpace();
		// create the data
		QByteArray nodes(rs);
		// pack the found nodes in a byte array
		if (rs > 0)
			kns.pack(nodes);
		
		FindNodeRsp fnr(r->getMTID(),node->getOurID(),nodes);
		fnr.setOrigin(r->getOrigin());
		srv->sendMsg(&fnr);
	}

	
	void DHT::announce(AnnounceReq* r)
	{
		if (!running)
			return;
		
		// ignore requests we get from ourself
		if (r->getID() == node->getOurID())
			return;
		
		Out(SYS_DHT|LOG_DEBUG) << "DHT: got announce request" << endl;
		node->recieved(this,r);
		// first check if the token is OK
		dht::Key token = r->getToken();
		if (!db->checkToken(token,r->getOrigin().ipAddress().IPv4Addr(),r->getOrigin().port()))
			return;
		
		// everything OK, so store the value
		Uint8 tdata[6];
		bt::WriteUint32(tdata,0,r->getOrigin().ipAddress().IPv4Addr());
		bt::WriteUint16(tdata,4,r->getPort());
		db->store(r->getInfoHash(),DBItem(tdata));
		// send a proper response to indicate everything is OK
		AnnounceRsp rsp(r->getMTID(),node->getOurID());
		rsp.setOrigin(r->getOrigin());
		srv->sendMsg(&rsp);
	}
	
	
	
	void DHT::getPeers(GetPeersReq* r)
	{
		if (!running)
			return;
		
		// ignore requests we get from ourself
		if (r->getID() == node->getOurID())
			return;
		
		Out(SYS_DHT|LOG_DEBUG) << "DHT: got getPeers request" << endl;
		node->recieved(this,r);
		DBItemList dbl;
		db->sample(r->getInfoHash(),dbl,50);
		
		// generate a token
		dht::Key token = db->genToken(r->getOrigin().ipAddress().IPv4Addr(),r->getOrigin().port());
		
		if (dbl.count() == 0)
		{
			// if data is null do the same as when we have a findNode request
			// find the K closest nodes and pack them
			KClosestNodesSearch kns(r->getInfoHash(),K);
			node->findKClosestNodes(kns);
			Uint32 rs = kns.requiredSpace();
			// create the data
			QByteArray nodes(rs);
			// pack the found nodes in a byte array
			if (rs > 0)
				kns.pack(nodes);
		
			GetPeersRsp fnr(r->getMTID(),node->getOurID(),nodes,token);
			fnr.setOrigin(r->getOrigin());
			srv->sendMsg(&fnr);
		}
		else
		{			
			// send a get peers response
			GetPeersRsp fvr(r->getMTID(),node->getOurID(),dbl,token);
			fvr.setOrigin(r->getOrigin());
			srv->sendMsg(&fvr);
		}
	}
	
	void DHT::response(MsgBase* r)
	{
		if (!running)
			return;
		
		node->recieved(this,r);
	}
	
	void DHT::error(ErrMsg* )
	{}
	

	void DHT::portRecieved(const QString & ip,bt::Uint16 port)
	{
		if (!running)
			return;
		
		Out(SYS_DHT|LOG_DEBUG) << "Sending ping request to " << ip << ":" << port << endl;
		PingReq* r = new PingReq(node->getOurID());
		r->setOrigin(KInetSocketAddress(ip,port));
		srv->doCall(r);
	}
	
	bool DHT::canStartTask() const
	{
		// we can start a task if we have less then  7 runnning and
		// there are at least 16 RPC slots available
		if (tman->getNumTasks() >= 7)
			return false;
		else if (256 - srv->getNumActiveRPCCalls() <= 16)
			return false;
		
		return true;	
	}
	
	AnnounceTask* DHT::announce(const bt::SHA1Hash & info_hash,bt::Uint16 port)
	{
		if (!running)
			return 0;
		
		KClosestNodesSearch kns(info_hash,K);
		node->findKClosestNodes(kns);
		if (kns.getNumEntries() > 0)
		{
			Out(SYS_DHT|LOG_NOTICE) << "DHT: Doing announce " << endl;
			AnnounceTask* at = new AnnounceTask(db,srv,node,info_hash,port);
			at->start(kns,!canStartTask());
			tman->addTask(at);
			if (!db->contains(info_hash))
				db->insert(info_hash);
			return at;
		}
		
		return 0;
	}
	
	NodeLookup* DHT::refreshBucket(const dht::Key & id,KBucket & bucket)
	{
		if (!running)
			return 0;
		
		KClosestNodesSearch kns(id,K);
		bucket.findKClosestNodes(kns);
		bucket.updateRefreshTimer();
		if (kns.getNumEntries() > 0)
		{
			Out(SYS_DHT|LOG_DEBUG) << "DHT: refreshing bucket " << endl;
			NodeLookup* nl = new NodeLookup(id,srv,node);
			nl->start(kns,!canStartTask());
			tman->addTask(nl);
			return nl;
		}
		
		return 0;
	}
	
	NodeLookup* DHT::findNode(const dht::Key & id)
	{
		if (!running)
			return 0;
		
		KClosestNodesSearch kns(id,K);
		node->findKClosestNodes(kns);
		if (kns.getNumEntries() > 0)
		{
			Out(SYS_DHT|LOG_DEBUG) << "DHT: finding node " << endl;
			NodeLookup* at = new NodeLookup(id,srv,node);
			at->start(kns,!canStartTask());
			tman->addTask(at);
			return at;
		}
		
		return 0;
	}
	
	void DHT::update()
	{
		if (!running)
			return;
		
		if (expire_timer.getElapsedSinceUpdate() > 5*60*1000)
		{
			db->expire(bt::GetCurrentTime());
			expire_timer.update();
		}
		
		node->refreshBuckets(this);
		tman->removeFinishedTasks(this);
		stats.num_tasks = tman->getNumTasks() + tman->getNumQueuedTasks();
		stats.num_peers = node->getNumEntriesInRoutingTable();
	}
	
	void DHT::timeout(const MsgBase* r)
	{
		node->onTimeout(r);
	}
	
	void DHT::addDHTNode(const QString & host,Uint16 hport)
	{
		if (!running)
			return;
		
		KResolverResults res = KResolver::resolve(host,QString::number(hport));
		if (res.count() > 0)
		{
			srv->ping(node->getOurID(),res.front().address());
		}
	}
	
	QMap<QString, int> DHT::getClosestGoodNodes(int maxNodes)
	{
		QMap<QString, int> map;
		
		if(!node)
			return map;
		
		int max = 0;
		KClosestNodesSearch kns(node->getOurID(), maxNodes*2);
		node->findKClosestNodes(kns);
		
		KClosestNodesSearch::Itr it;
		for(it = kns.begin(); it != kns.end(); ++it)
		{
			KBucketEntry e = it->second;
			
			if(!e.isGood())
				continue;
			
			KInetSocketAddress a = e.getAddress();
			
			map.insert(a.ipAddress().toString(), a.port());
			if(++max >= maxNodes)
				break;
		}
		
		return map;
	}
}

#include "dht.moc"
