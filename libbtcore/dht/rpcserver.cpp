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
#include "rpcserver.h"
#include <QHostAddress>
#include <QThread>
#include <unistd.h>
#include <string.h>
#include <net/portlist.h>
#include <util/log.h>
#include <util/error.h>
#include <torrent/globals.h>
#include <bcodec/bnode.h>
#include <bcodec/bdecoder.h>
#include <bcodec/bencoder.h>
#include <util/functions.h>
#include "rpccall.h"
#include "rpcmsg.h"
#include "kbucket.h"
#include "node.h"
#include "dht.h"
#include <net/socket.h>




using namespace KNetwork;
using namespace bt;

namespace dht
{
	class RPCServerThread : public QThread
	{
	public:
		RPCServerThread(net::Socket* socket,RPCServer* rpc) : running(false),socket(socket),rpc(rpc)
		{
		}
		
		virtual ~RPCServerThread()
		{
			qDeleteAll(incoming);
		}
		
		void handlePacket()
		{
			QMutexLocker lock(&mutex);
			QByteArray data(socket->bytesAvailable(),0);
			net::Address addr;
			if (socket->recvFrom((bt::Uint8*)data.data(),data.size(),addr) <= 0)
				return;
			
			
			BNode* n = 0;
			try
			{
				// read and decode the packet
				BDecoder bdec(data,false);
				n = bdec.decode();
				
				if (!n || n->getType() != BNode::DICT)
				{
					delete n;
					return;
				}
				
				// try to make a RPCMsg of it
				MsgBase* msg = MakeRPCMsg((BDictNode*)n,rpc);
				if (msg)
				{
					msg->setOrigin(addr);
					incoming.append(msg);
				}
			}
			catch (bt::Error & err)
			{
				Out(SYS_DHT|LOG_IMPORTANT) << "Error happened during parsing : " << err.toString() << endl;
			}
			delete n;
		}
		
		virtual void run()
		{
			running = true;
			fd_set fds;
			FD_ZERO(&fds);
			while (running)
			{
				int fd = socket->fd();
				FD_SET(fd,&fds);
				struct timeval tv = {0,500000};
				if (select(fd + 1,&fds,0,0,&tv) > 0)
				{
					handlePacket();
				}
			}
		}
		
		void stop()
		{
			running = false;
		}
		
		MsgBase* nextMessage()
		{
			if (incoming.isEmpty())
				return 0;
			
			MsgBase* msg = incoming.first();
			incoming.pop_front();
			return msg;
		}
		
		QList<MsgBase*> incoming;
		QMutex mutex;
		bool running;
		net::Socket* socket;
		RPCServer* rpc;
	};
	


	RPCServer::RPCServer(DHT* dh_table,Uint16 port,QObject *parent) : QObject(parent),dh_table(dh_table),next_mtid(0),port(port)
	{
		sock = 0;
		listener_thread = 0;
	}


	RPCServer::~RPCServer()
	{
		bt::Globals::instance().getPortList().removePort(port,net::UDP);
		calls.setAutoDelete(true);
		calls.clear();
		qDeleteAll(call_queue);
		call_queue.clear();
	}
	
	void RPCServer::start()
	{
		QString ip = NetworkInterfaceIPAddress(NetworkInterface());
		
		QStringList possible;
		if (!ip.isEmpty())
			possible << ip;
		
		// If the first address doesn't work try AnyIPv6 and Any
		possible << QHostAddress(QHostAddress::AnyIPv6).toString() << QHostAddress(QHostAddress::Any).toString();
		
		foreach (const QString & addr,possible)
		{
			net::Address address(addr,port);
			sock = new net::Socket(false,address.ipVersion());
			if (!sock->bind(addr,port,false))
			{
				Out(SYS_DHT|LOG_IMPORTANT) << "DHT: Failed to bind to " << addr << ":" << port << endl;
				delete sock;
				sock = 0;
			}
			else
			{
				Out(SYS_DHT|LOG_NOTICE) << "DHT: Bound to " << addr << ":" << port << endl;
				bt::Globals::instance().getPortList().addNewPort(port,net::UDP,true);
				break;
			}
		}
		
		if (sock)
		{
			listener_thread = new RPCServerThread(sock,this);
			listener_thread->start(QThread::IdlePriority);
		}
	}
		
	void RPCServer::stop()
	{
		bt::Globals::instance().getPortList().removePort(port,net::UDP);
		if (listener_thread)
		{
			listener_thread->stop();
			listener_thread->wait();
			delete listener_thread;
			listener_thread = 0;
		}
		
		if (sock)
		{
			sock->close();
			delete sock;
			sock = 0;
		}
	}
	
#if 0
	static void PrintRawData(const QByteArray & data)
	{
		QString tmp;
		for (int i = 0;i < data.size();i++)
		{
			char c = QChar(data[i]).toLatin1();
			if (!QChar(data[i]).isPrint() || c == 0)
				tmp += '#';
			else
				tmp += c;
		}
		
		Out(SYS_DHT|LOG_DEBUG) << tmp << endl;
	}
#endif

	void RPCServer::handlePackets()
	{
		if (!listener_thread)
			return; 
		
		// lock the thread
		QMutexLocker lock(&listener_thread->mutex);
		
		MsgBase* msg = 0;
		while ((msg = listener_thread->nextMessage()) != 0)
		{
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
	
	
	void RPCServer::send(const net::Address & addr,const QByteArray & msg)
	{
		if (sock)
			sock->sendTo((const bt::Uint8*)msg.data(),msg.size(),addr);
	}
	
	RPCCall* RPCServer::doCall(MsgBase* msg)
	{
		QMutexLocker lock(&mutex);
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
		QMutexLocker lock(&mutex);
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
	
	Method RPCServer::findMethod(Uint8 mtid)
	{
		QMutexLocker lock(&mutex);
		const RPCCall* call = calls.find(mtid);
		if (call)
			return call->getMsgMethod();
		else
			return dht::NONE;
	}
	
	void RPCServer::ping(const dht::Key & our_id,const net::Address & addr)
	{
		Out(SYS_DHT|LOG_NOTICE) << "DHT: pinging " << addr.nodeName() << endl;
		PingReq* pr = new PingReq(our_id);
		pr->setOrigin(addr);
		doCall(pr);
	}
	

}
#include "rpcserver.moc"
