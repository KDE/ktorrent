/***************************************************************************
 *   Copyright (C) 2009 by Joris Guisson                                   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#include "utpserver.h"
#include <stdlib.h>
#include <sys/select.h>
#include <time.h>
#include <util/log.h>
#include <util/constants.h>
#include "utpprotocol.h"
#include "utpserverthread.h"

#ifdef Q_WS_WIN
#include <util/win32.h>
#endif

using namespace bt;

namespace utp
{
	UTPServer::UTPServer(QObject* parent) : ServerInterface(parent),sock(0),running(false),utp_thread(0),mutex(QMutex::Recursive)
	{
		qsrand(time(0));
		connections.setAutoDelete(true);
	}

	UTPServer::~UTPServer()
	{
		stop();
	}
	
	
	bool UTPServer::changePort(bt::Uint16 port)
	{
		QStringList possible = bindAddresses();
		foreach (const QString & ip,possible)
		{
			net::Address addr(ip,port);
			if (bind(addr))
				return true;
		}
		
		return false;
	}


	bool UTPServer::bind(const net::Address& addr)
	{
		sock = new net::Socket(false,addr.ipVersion());
		if (!sock->bind(addr,false))
		{
			delete sock;
			sock = 0;
			return false;
		}
		
		Out(SYS_CON|LOG_NOTICE) << "UTP: bound to " << addr.toString() << endl;
		return true;
	}

	void UTPServer::run()
	{
		running = true;
		fd_set fds;
		FD_ZERO(&fds);
		while (running)
		{
			int fd = sock->fd();
			FD_SET(fd,&fds);
			struct timeval tv = {0,100000};
			if (select(fd + 1,&fds,0,0,&tv) > 0)
			{
				handlePacket();
			}
			
			checkTimeouts();
			clearDeadConnections();
		}
	}
	
	void UTPServer::handlePacket()
	{
		QMutexLocker lock(&mutex);
		
		int ba = sock->bytesAvailable();
		QByteArray packet(ba,0);
		net::Address addr;
		if (sock->recvFrom((bt::Uint8*)packet.data(),ba,addr) > 0)
		{
			Out(SYS_CON|LOG_NOTICE) << "UTP: received " << ba << " bytes packet from " << addr.toString() << endl;
			// discard packets which are to small
			if (ba < (int)sizeof(utp::Header))
				return;
			
			Header* hdr = (Header*)packet.data();
			switch (hdr->type)
			{
			case ST_DATA:
			case ST_FIN:
			case ST_STATE:
				try
				{
					Connection* c = find(hdr->connection_id);
					if (c)
						c->handlePacket(packet);
					else
						Out(SYS_CON|LOG_NOTICE) << "UTP: unkown connection " << hdr->connection_id << endl;
				}
				catch (Connection::TransmissionError & err)
				{
					Out(SYS_CON|LOG_NOTICE) << "UTP: " << err.location << endl;
					// TODO: kill connection
				}
				break;
			case ST_RESET:
				reset(hdr);
				break;
			case ST_SYN:
				syn(hdr,packet,addr);
				break;
			}
		}
	}

	bool UTPServer::sendTo(const QByteArray& data, const net::Address& addr)
	{
		return sock->sendTo((const bt::Uint8*)data.data(),data.size(),addr) == data.size();
	}
	
	bool UTPServer::sendTo(const bt::Uint8* data, const bt::Uint32 size, const net::Address& addr)
	{
		return sock->sendTo(data,size,addr) == (int)size;
	}

	Connection* UTPServer::connectTo(const net::Address& addr)
	{
		QMutexLocker lock(&mutex);
		quint16 recv_conn_id = qrand() % 32535;
		while (connections.contains(recv_conn_id))
			recv_conn_id = qrand() % 32535;
		
		Connection* conn = new Connection(recv_conn_id,Connection::OUTGOING,addr,this);
		connections.insert(recv_conn_id,conn);
		
		Out(SYS_CON|LOG_NOTICE) << "UTP: connecting to " << addr.toString() << endl;
		conn->startConnecting();
		return conn;
	}

	void UTPServer::syn(const utp::Header* hdr, const QByteArray& data, const net::Address & addr)
	{
		quint16 recv_conn_id = hdr->connection_id + 1;
		if (connections.find(recv_conn_id))
		{
			// Send a reset packet if the ID is in use
			Connection conn(recv_conn_id,Connection::INCOMING,addr,this);
			conn.sendReset();
		}
		else
		{
			Connection* conn = new Connection(recv_conn_id,Connection::INCOMING,addr,this);
			try
			{
				conn->handlePacket(data);
				connections.insert(recv_conn_id,conn);
				accepted(conn);
			}
			catch (Connection::TransmissionError & err)
			{
				Out(SYS_CON|LOG_NOTICE) << "UTP: " << err.location << endl;
				delete conn;
			}
		}
	}

	void UTPServer::reset(const utp::Header* hdr)
	{
		Connection* c = find(hdr->connection_id);
		if (c)
			kill(c);
	}

	Connection* UTPServer::find(quint16 conn_id)
	{
		return connections.find(conn_id);
	}

	void UTPServer::kill(Connection* conn)
	{
		QMutexLocker lock(&mutex);
		dead_connections.append(conn);
	}
	
	void UTPServer::clearDeadConnections()
	{
		QMutexLocker lock(&mutex);
		foreach (utp::Connection* conn,dead_connections)
		{
			connections.erase(conn->receiveConnectionID());
		}
		dead_connections.clear();
	}

	void UTPServer::stop()
	{
		running = false;
		if (utp_thread)
		{
			utp_thread->wait();
			delete utp_thread;
			utp_thread = 0;
		}
	}
	
	
	void UTPServer::start()
	{
		if (!utp_thread)
		{
			utp_thread = new UTPServerThread(this);
			utp_thread->start();
		}
	}

	void UTPServer::checkTimeouts()
	{
		QMutexLocker lock(&mutex);
		ConItr itr = connections.begin();
		while (itr != connections.end())
		{
			(*itr).second->checkTimeout();
			itr++;
		}
	}


}
