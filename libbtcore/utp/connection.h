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

#include <QPair>
#include <QMutex>
#include <QWaitCondition>
#include <QBasicTimer>
#include <btcore_export.h>
#include <net/address.h>
#include <utp/utpprotocol.h>
#include <util/circularbuffer.h>
#include <util/timer.h>
#include <utp/remotewindow.h>




namespace utp
{
	class DelayWindow;;
	class LocalWindow;
	
	
	
	/**
		Interface class for transmitting packets
	*/
	class BTCORE_EXPORT Transmitter
	{
	public:
		virtual ~Transmitter() {}
		
		/// Send a packet to some host
		virtual bool sendTo(const QByteArray & data,const net::Address & addr,quint16 conn_id) = 0;
	};

	/**
		Keeps track of a single UTP connection
	*/
	class BTCORE_EXPORT Connection : public QObject, public Retransmitter
	{
		Q_OBJECT
	public:
		enum Type
		{
			INCOMING,
			OUTGOING
		};
		
		/// Thrown when a transmission error occurs, server should kill the connection if it happens
		class TransmissionError 
		{
		public:
			TransmissionError(const char* file,int line);
			
			QString location;
		};
		
		struct Stats
		{
			Type type;
			net::Address remote;
			ConnectionState state;
			bt::Uint16 send_connection_id;
			bt::Uint32 reply_micro;
			bt::Uint16 recv_connection_id;
			bt::Uint16 seq_nr;
			int eof_seq_nr;
			bt::Uint32 timeout;
			int rtt;
			int rtt_var;
			bt::Uint32 packet_size;
			bt::Uint32 last_window_size_transmitted;
			
			bt::Uint64 bytes_received;
			bt::Uint64 bytes_sent;
			bt::Uint32 packets_received;
			bt::Uint32 packets_sent;
			bt::Uint64 bytes_lost;
			bt::Uint32 packets_lost;
		};
		
		Connection(bt::Uint16 recv_connection_id,Type type,const net::Address & remote,Transmitter* transmitter);
		virtual ~Connection();
		
		/// Dump connection stats
		void dumpStats();
		
		/// Start connecting (OUTGOING only)
		void startConnecting();
		
		/// Get the connection stats
		const Stats & connectionStats() const {return stats;}
		
		/// Handle a single packet
		ConnectionState handlePacket(const PacketParser & parser,const QByteArray & packet);
		
		/// Get the remote address
		const net::Address & remoteAddress() const {return stats.remote;}
		
		/// Get the receive connection id
		bt::Uint16 receiveConnectionID() const {return stats.recv_connection_id;}
		
		/// Send some data, returns the amount of bytes sent (or -1 on error)
		int send(const bt::Uint8* data,bt::Uint32 len);
		
		/// Read available data from local window, returns the amount of bytes read
		int recv(bt::Uint8* buf,bt::Uint32 max_len);
		
		/// Get the connection state
		ConnectionState connectionState() const {return stats.state;}
		
		/// Get the type of connection
		Type connectionType() const {return stats.type;}
		
		/// Get the number of bytes available
		bt::Uint32 bytesAvailable() const;
		
		/// Can we write to this socket
		bool isWriteable() const;
		
		/// Wait until the connectTo call fails or succeeds
		bool waitUntilConnected();
		
		/// Wait until there is data ready or the socket is closed
		bool waitForData();
		
		/// Close the socket
		void close();
		
		/// Reset the connection
		void reset();
		
		/// Update the RTT time
		virtual void updateRTT(const Header* hdr,bt::Uint32 packet_rtt,bt::Uint32 packet_size);
		
		/// Retransmit a packet
		virtual void retransmit(const QByteArray & packet,bt::Uint16 p_seq_nr);
		
		/// Is all data sent
		bool allDataSent() const;
		
		/// Get the current timeout
		virtual bt::Uint32 currentTimeout() const {return stats.timeout;}
		
	private:
		void sendSYN();
		void sendState();
		void sendFIN();
		void sendReset();
		void updateDelayMeasurement(const Header* hdr);
		void sendStateOrData();
		void sendPackets();
		void sendPacket(bt::Uint32 type,bt::Uint16 p_ack_nr);
		void checkIfClosed();
		int sendDataPacket(const QByteArray & packet);
		virtual void timerEvent(QTimerEvent* event);
		void handleTimeout();
		void startTimer();
		
	private slots:
		void delayedStartTimer();
		
	signals:
		void doDelayedStartTimer();
		
	private:
		Transmitter* transmitter;
		LocalWindow* local_wnd;
		RemoteWindow* remote_wnd;
		bt::CircularBuffer output_buffer;
		//bt::Timer timer;
		mutable QMutex mutex;
		QWaitCondition connected;
		QWaitCondition data_ready;
		Stats stats;
		bool fin_sent;
		TimeValue last_packet_sent;
		DelayWindow* delay_window;
		QBasicTimer timer;
		
		friend class UTPServer;
	};
}

#endif // UTP_CONNECTION_H
