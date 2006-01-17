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
#include "rpcserver.h"
#include "rpccall.h"
#include "kbucket.h"

namespace dht
{

	RPCServer::RPCServer(Uint16 port,QObject *parent) : QObject(parent)
	{
		sock = new KDatagramSocket(this);
		sock->setBlocking(false);
		connect(sock,SIGNAL(readyRead()),this,SLOT(readPacket()));
		sock->bind(QString::null,QString::number(port));
	}


	RPCServer::~RPCServer()
	{
		sock->close();
	}

	void RPCServer::readPacket()
	{
	}

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


}
#include "rpcserver.moc"
