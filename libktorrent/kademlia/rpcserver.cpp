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
#include <string.h>
#include <util/log.h>
#include <util/error.h>
#include <torrent/globals.h>
#include <torrent/bnode.h>
#include <torrent/bdecoder.h>
#include <torrent/bencoder.h>
#include "rpcserver.h"
#include "rpccall.h"
#include "rpcmsg.h"
#include "kbucket.h"
#include "node.h"

using namespace KNetwork;
using namespace bt;

namespace dht
{
	


	RPCServer::RPCServer(DHT* dh_table,Uint16 port,QObject *parent) : QObject(parent),dh_table(dh_table),next_mtid(0)
	{
		sock = new KDatagramSocket(this);
		sock->setBlocking(false);
		sock->setAddressReuseable(true);
		connect(sock,SIGNAL(readyRead()),this,SLOT(readPacket()));
		sock->bind(QString::null,QString::number(port));
	}


	RPCServer::~RPCServer()
	{
		sock->close();
		calls.setAutoDelete(true);
		calls.clear();
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
		
		Out() << tmp << endl;
	}

	void RPCServer::readPacket()
	{
		Out() << "RPCServer::readPacket" << endl;
		KDatagramPacket pck = sock->receive();
		PrintRawData(pck.data());
		BNode* n = 0;
		try
		{
		// read and decode the packet
			BDecoder bdec(pck.data(),false);	
			n = bdec.decode();
			if (n->getType() != BNode::DICT)
			{
				delete n;
				return;
			}
			
			// try to make a RPCMsg of it
			MsgBase* msg = MakeRPCMsg((BDictNode*)n,this);
			if (!msg)
			{
				Out() << "Error parsing message : " << endl;
				PrintRawData(pck.data());
				return;
			}
			else
			{
				msg->setOrigin(pck.address());
				msg->print();
				msg->apply(dh_table);
			// erase an existing call
				if (msg->getType() == RSP_MSG && calls.contains(msg->getMTID()))
				{
				// delete the call, but first notify it off the response
					RPCCall* c = calls.find(msg->getMTID());
					c->response(msg);
					calls.erase(msg->getMTID());
					delete c;
				}
				delete msg;
			}
		}
		catch (bt::Error & err)
		{
			Out() << "Error happened during parsing : " << err.toString() << endl;
		}
		delete n;
	}
	
	
	void RPCServer::send(const KNetwork::KSocketAddress & addr,const QByteArray & msg)
	{
		sock->send(KNetwork::KDatagramPacket(msg,addr));
	}
	
	RPCCall* RPCServer::doCall(MsgBase* msg)
	{
		while (calls.contains(next_mtid))
			next_mtid++;
		
		msg->setMTID(next_mtid++);
		sendMsg(msg);
		RPCCall* c = new RPCCall(this,msg);
		calls.insert(msg->getMTID(),c);
		return c;
	}
	
	void RPCServer::sendMsg(MsgBase* msg)
	{
		QByteArray data;
		msg->encode(data);
		send(msg->getOrigin(),data);
		
		PrintRawData(data);
	}
	
	void RPCServer::timedOut(Uint8 mtid)
	{
		// delete the call
		RPCCall* c = calls.find(mtid);
		if (c)
		{
			calls.erase(mtid);
			c->deleteLater();
		}
	}
	
	const RPCCall* RPCServer::findCall(Uint8 mtid) const
	{
		return calls.find(mtid);
	}
	
	

}
#include "rpcserver.moc"
