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

#ifndef UTP_CONNECTION_H
#define UTP_CONNECTION_H

#include <QMutex>
#include <QWaitCondition>
#include <btcore_export.h>
#include <net/address.h>
#include "utpprotocol.h"



namespace utp
{
	class RemoteWindow;
	class LocalWindow;
	class UTPServer;

	/**
		Keeps track of a single UTP connection
	*/
	class BTCORE_EXPORT Connection
	{
	public:
		enum Type
		{
			INCOMING,
			OUTGOING
		};
		Connection(bt::Uint16 recv_connection_id,Type type,const net::Address & remote,UTPServer* srv);
		virtual ~Connection();
		
		/// Handle a single packet
		ConnectionState handlePacket(const QByteArray & packet);
		
		/// Get the remote address
		const net::Address & remoteAddress() const {return remote;}
		
		/// Get the receive connection id
		bt::Uint16 receiveConnectionID() const {return recv_connection_id;}
		
		/// Send a packet, will return false if there is no room in the remote window
		bool send(const QByteArray & packet);
		
		/// Send some data, returns the amount of bytes sent
		bt::Uint32 send(const bt::Uint8* data,bt::Uint32 len);
		
		/// Read available data from local window, returns the amount of bytes read
		bt::Uint32 recv(bt::Uint8* buf,bt::Uint32 max_len);
		
		/// Get the connection state
		ConnectionState connectionState() const {return state;}
		
		/// Get the type of connection
		Type connectionType() const {return type;}
		
		/// Send a reset packet
		void sendReset();
		
		/// Get the number of bytes available
		bt::Uint32 bytesAvailable() const;
		
		/// Wait until the connectTo call fails or succeeds
		bool waitUntilConnected();
		
		/// Wait until there is data ready or the socket is closed
		bool waitForData();
		
		/// Close the socket
		void close();
		
	private:
		void sendSYN();
		void waitForSYN();
		void sendState();
		void sendFIN();
		void updateDelayMeasurement(const Header* hdr);
		
		/** 
			Parses the packet, and retrieves pointer to the header, the SelectiveAck extension (if present)
			@param packet The packet
			@param hdr The header pointer
			@param selective_ack The SelectiveAck pointer
			@return The offset of the data or -1 if there is no data
		*/
		int parsePacket(const QByteArray & packet,Header** hdr,SelectiveAck** selective_ack);
		
	private:
		Type type;
		UTPServer* srv;
		net::Address remote;
		
		ConnectionState state;
		bt::Uint16 send_connection_id;
		bt::Uint32 reply_micro;
		
		bt::Uint16 recv_connection_id;
		LocalWindow* local_wnd;
		RemoteWindow* remote_wnd;
		
		bt::Uint16 seq_nr;
		bt::Uint16 ack_nr;
		int eof_seq_nr;
		
		mutable QMutex mutex;
		QWaitCondition connected;
		QWaitCondition data_ready;
	};
}

#endif // UTP_CONNECTION_H
