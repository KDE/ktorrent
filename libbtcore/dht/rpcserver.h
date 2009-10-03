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
#ifndef DHTRPCSERVER_H
#define DHTRPCSERVER_H

#include <QList>
#include <QObject>
#include <dht/rpcmsg.h>
#include <net/address.h>
#include <util/constants.h>
#include <util/ptrmap.h>
#include <QMutex>

namespace net
{
	class Socket;
}

using bt::Uint32;
using bt::Uint16;
using bt::Uint8;

namespace dht
{
	class RPCServerThread;
	class Key;
	class RPCCall;
	class DHT;

	/**
	 * @author Joris Guisson
	 *
	 * Class to handle incoming and outgoing RPC messages.
	 */
	class RPCServer : public QObject
	{
		Q_OBJECT
	public:
		RPCServer(DHT* dh_table,Uint16 port,QObject *parent = 0);
		virtual ~RPCServer();
		
		/// Start the server
		void start();
		
		/// Stop the server
		void stop();
		
		/**
		 * Do a RPC call.
		 * @param msg The message to send
		 * @return The call object
		 */
		RPCCall* doCall(MsgBase* msg);
		
		/**
		 * Send a message, this only sends the message, it does not keep any call
		 * information. This should be used for replies.
		 * @param msg The message to send
		 */
		void sendMsg(MsgBase* msg);
		
		
		/**
		 * A call was timed out.
		 * @param mtid mtid of call
		 */
		void timedOut(Uint8 mtid);
		
		/**
		 * Ping a node, we don't care about the MTID.
		 * @param addr The address
		 */
		void ping(const dht::Key & our_id,const net::Address & addr);
		
		/// Get the number of active calls
		Uint32 getNumActiveRPCCalls() const {return calls.count();}
	
		/// Handle all incoming packets
		void handlePackets();
		
		/// Find the method given an mtid
		Method findMethod(Uint8 mtid);
		
	private:
		void send(const net::Address & addr,const QByteArray & msg);
		void doQueuedCalls();
			
	private:
		RPCServerThread* listener_thread;
		net::Socket* sock;
		DHT* dh_table;
		bt::PtrMap<bt::Uint8,RPCCall> calls;
		QList<RPCCall*> call_queue;
		bt::Uint8 next_mtid;
		bt::Uint16 port;
		QMutex mutex;
	};

}

#endif
