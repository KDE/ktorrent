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
	


	RPCServer::RPCServer(Node* node,Uint16 port,QObject *parent) : QObject(parent),node(node)
	{
		sock = new KDatagramSocket(this);
		sock->setBlocking(false);
		connect(sock,SIGNAL(readyRead()),this,SLOT(readPacket()));
		sock->bind(QString::null,QString::number(port));
		mtid = 0;
	}


	RPCServer::~RPCServer()
	{
		sock->close();
	}

	void RPCServer::readPacket()
	{
		Out() << "RPCServer::readPacket" << endl;
		KDatagramPacket pck = sock->receive();
		
		// read and decode the packet
		BDecoder bdec(pck.data(),false);
		
		BNode* n = bdec.decode();
		n->printDebugInfo();
		if (n->getType() != BNode::DICT)
		{
			delete n;
			return;
		}
		
		// try to make a RPCMsg of it
		RPCMsg msg = RPCMsg::parse((BDictNode*)n);
		delete node;
		if (!msg.isValid())
			return;
	
		msg.setOrigin(pck.address());
		// we now have a valid packet, so we must update our buckets
		node->recieved(msg);
			
		switch (msg.getType())
		{
			case RPCMsg::REQ_MSG:
				handleReq(msg);
				break;
			case RPCMsg::RSP_MSG:
				handleRsp(msg);
				break;
			case RPCMsg::ERR_MSG:
				handleErr(msg);
			case RPCMsg::NONE:
			default:
				break;
		}
		
	}
	
	void RPCServer::handleReq(const RPCMsg & msg)
	{
		switch (msg.getMethod())
		{
			case RPCMsg::PING:
				break;
			case RPCMsg::FIND_NODE:
			case RPCMsg::FIND_VALUE:
			case RPCMsg::STORE_VALUE:
			case RPCMsg::STORE_VALUES:
				break;
			case RPCMsg::INVALID:
			default:
				break;
		}
	}
	
	void RPCServer::handleRsp(const RPCMsg & msg)
	{
	}
	
	void RPCServer::handleErr(const RPCMsg & msg)
	{
	}

	/*
	
	void sendRequest(const QString & method)
	{
		QByteArray data;
		BEncoder enc(new BEncoderBufferOutput(data));
	}
	*/
	/*
	RPCCall* RPCServer::ping(const KBucketEntry & to)
	{
		return 0;
	}
	
	RPCCall* RPCServer::findNode(const KBucketEntry & to,const Key & k)
	{
		return 0;
	}
	
	RPCCall* RPCServer::findValue(const KBucketEntry & to,const Key & k)
	{
		return 0;
	}
	
	RPCCall* RPCServer::store(const KBucketEntry & to,const Key & k,const bt::Array<Uint8> & data)
	{
		return 0;
	}
	*/

}
#include "rpcserver.moc"
