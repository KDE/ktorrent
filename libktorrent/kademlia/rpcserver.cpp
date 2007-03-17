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
#include <unistd.h>
#include <string.h>
#include <net/portlist.h>
#include <util/log.h>
#include <util/error.h>
#include <torrent/globals.h>
#include <torrent/bnode.h>
#include <torrent/bdecoder.h>
#include <torrent/bencoder.h>
#include <ksocketdevice.h>
#include "rpcserver.h"
#include "rpccall.h"
#include "rpcmsg.h"
#include "kbucket.h"
#include "node.h"
#include "dht.h"

using namespace KNetwork;
using namespace bt;

namespace dht
{
	


	RPCServer::RPCServer(DHT* dh_table,Uint16 port,QObject *parent) : QObject(parent),dh_table(dh_table),next_mtid(0),port(port)
	{
		sock = new KDatagramSocket(this);
		sock->setBlocking(false);
		sock->setAddressReuseable(true);
	}


	RPCServer::~RPCServer()
	{
		bt::Globals::instance().getPortList().removePort(port,net::UDP);
		sock->close();
		calls.setAutoDelete(true);
		calls.clear();
		call_queue.setAutoDelete(true);
		call_queue.clear();
	}
	
	void RPCServer::start()
	{
		sock->setBlocking(true);
		if (!sock->bind(QString::null,QString::number(port)))
		{
			Out(SYS_DHT|LOG_IMPORTANT) << "DHT: Failed to bind to UDP port " << port << " for DHT" << endl;
		}
		else
		{
			bt::Globals::instance().getPortList().addNewPort(port,net::UDP,true);
		}
		sock->setBlocking(false);
		connect(sock,SIGNAL(readyRead()),this,SLOT(readPacket()));
	}
		
	void RPCServer::stop()
	{
		bt::Globals::instance().getPortList().removePort(port,net::UDP);
		sock->close();
	}
	
	static void PrintRawData(const QByteArray & data)
	{
		QString tmp;
		for (Uint32 i = 0;i < data.size();i++)
		{
			char c = QChar(data[i]).latin1();
			if (!QChar(data[i]).isPrint() || c == 0)
				tmp += '#';
			else
				tmp += c;
		}
		
		Out(SYS_DHT|LOG_DEBUG) << tmp << endl;
	}

	void RPCServer::readPacket()
	{
		if (sock->bytesAvailable() == 0)
		{
			Out(SYS_DHT|LOG_NOTICE) << "0 byte UDP packet " << endl;
			// KDatagramSocket wrongly handles UDP packets with no payload
			// so we need to deal with it oursleves
			int fd = sock->socketDevice()->socket();
			char tmp;
			read(fd,&tmp,1);
			return;
		}
		
		KDatagramPacket pck = sock->receive();
		/*
		Out() << "RPCServer::readPacket" << endl;
		PrintRawData(pck.data());
		*/
		BNode* n = 0;
		try
		{
		// read and decode the packet
			BDecoder bdec(pck.data(),false);	
			n = bdec.decode();
			
			if (!n || n->getType() != BNode::DICT)
			{
				delete n;
				return;
			}
			
			// try to make a RPCMsg of it
			MsgBase* msg = MakeRPCMsg((BDictNode*)n,this);
			if (msg)
			{
				msg->setOrigin(pck.address());
				msg->apply(dh_table);
			// erase an existing call
				if (msg->getType() == RSP_MSG && calls.contains(msg->getMTID()))
				{
				// delete the call, but first notify it off the response
					RPCCall* c = calls.find(msg->getMTID());
					c->response(msg);
					calls.erase(msg->getMTID());
					c->deleteLater();
					doQueuedCalls();
				}
				delete msg;
			}
		}
		catch (bt::Error & err)
		{
			Out(SYS_DHT|LOG_IMPORTANT) << "Error happened during parsing : " << err.toString() << endl;
		}
		delete n;
		
		if (sock->bytesAvailable() > 0)
			readPacket();
	}
	
	
	void RPCServer::send(const KNetwork::KSocketAddress & addr,const QByteArray & msg)
	{
		sock->send(KNetwork::KDatagramPacket(msg,addr));
	}
	
	RPCCall* RPCServer::doCall(MsgBase* msg)
	{
		Uint8 start = next_mtid;
		while (calls.contains(next_mtid))
		{
			next_mtid++;
			if (next_mtid == start) // if this happens we cannot do any calls
			{
				// so queue the call
				RPCCall* c = new RPCCall(this,msg,true);
				call_queue.append(c);
				Out(SYS_DHT|LOG_NOTICE) << "Queueing RPC call, no slots available at the moment" << endl;
				return c; 
			}
		}
		
		msg->setMTID(next_mtid++);
		sendMsg(msg);
		RPCCall* c = new RPCCall(this,msg,false);
		calls.insert(msg->getMTID(),c);
		return c;
	}
	
	void RPCServer::sendMsg(MsgBase* msg)
	{
		QByteArray data;
		msg->encode(data);
		send(msg->getDestination(),data);
		
	//	PrintRawData(data);
	}
	
	void RPCServer::timedOut(Uint8 mtid)
	{
		// delete the call
		RPCCall* c = calls.find(mtid);
		if (c)
		{
			dh_table->timeout(c->getRequest());
			calls.erase(mtid);
			c->deleteLater();
		}
		doQueuedCalls();	
	}
	
	void RPCServer::doQueuedCalls()
	{
		while (call_queue.count() > 0 && calls.count() < 256)
		{
			RPCCall* c = call_queue.first();
			call_queue.removeFirst();
			
			while (calls.contains(next_mtid))
				next_mtid++;
			
			MsgBase* msg = c->getRequest();
			msg->setMTID(next_mtid++);
			sendMsg(msg);
			calls.insert(msg->getMTID(),c);
			c->start();
		}
	}
	
	const RPCCall* RPCServer::findCall(Uint8 mtid) const
	{
		return calls.find(mtid);
	}
	
	void RPCServer::ping(const dht::Key & our_id,const KNetwork::KSocketAddress & addr)
	{
		Out(SYS_DHT|LOG_NOTICE) << "DHT: pinging " << addr.nodeName() << endl;
		PingReq* pr = new PingReq(our_id);
		pr->setOrigin(addr);
		doCall(pr);
	}
	

}
#include "rpcserver.moc"
