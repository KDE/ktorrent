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

#ifndef UTP_UTPSERVER_H
#define UTP_UTPSERVER_H

#include <QThread>
#include <net/socket.h>
#include <util/ptrmap.h>
#include <interfaces/serverinterface.h>
#include <btcore_export.h>
#include "connection.h"



namespace utp
{
	class UTPServerThread;

	class BTCORE_EXPORT UTPServer : public bt::ServerInterface,public Transmitter
	{
		Q_OBJECT
	public:
		UTPServer(QObject* parent = 0);
		virtual ~UTPServer();
		
		virtual bool changePort(bt::Uint16 port);
		
		/// Send a packet to some host
		virtual bool sendTo(const QByteArray & data,const net::Address & addr);
		
		/// Send a packet to some host
		virtual bool sendTo(const bt::Uint8* data,const bt::Uint32 size,const net::Address & addr);
		
		/// Called by Connection when it needs to be killed
		void kill(Connection* conn);
		
		/// Setup a connection to a remote address
		Connection* connectTo(const net::Address & addr);
		
		/// Start the UTP server
		void start();
		
		/// Stop the UTP server
		void stop();
		
		/// Run the UTPServer
		void run();
		
	protected:
		bool bind(const net::Address & addr);
		void handlePacket();
		void syn(const Header* hdr,const QByteArray & data,const net::Address & addr);
		void reset(const Header* hdr);
		void clearDeadConnections();
		void checkTimeouts();
		Connection* find(quint16 conn_id);
		
	signals:
		void accepted(Connection* conn);
		
	private:
		net::Socket* sock;
		bool running;
		bt::PtrMap<quint16,Connection> connections;
		QList<Connection*> dead_connections;
		UTPServerThread* utp_thread;
		QMutex mutex;
		
		typedef bt::PtrMap<quint16,Connection>::iterator ConItr;
	};

}

#endif // UTP_UTPSERVER_H
