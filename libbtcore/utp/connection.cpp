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

#include "connection.h"
#include "utpserver.h"
#include <sys/time.h>
#include <util/log.h>
#include "localwindow.h"
#include "remotewindow.h"

using namespace bt;

namespace utp
{
	Connection::Connection(bt::Uint16 recv_connection_id, Type type, const net::Address& remote, UTPServer* srv) 
		: type(type),srv(srv),remote(remote),recv_connection_id(recv_connection_id)
	{
		reply_micro = 0;
		eof_seq_nr = -1;
		local_wnd = new LocalWindow();
		remote_wnd = new RemoteWindow();
		if (type == OUTGOING)
		{
			send_connection_id = recv_connection_id + 1;
			sendSYN();
		}
		else
		{
			send_connection_id = recv_connection_id - 1;
			waitForSYN();
		}
		
		Out(SYS_CON|LOG_NOTICE) << "UTP: Connection " << recv_connection_id << "|" << send_connection_id << endl;
	}

	Connection::~Connection()
	{
		delete local_wnd;
		delete remote_wnd;
	}
	
	static void DumpHeader(const Header & hdr)
	{
		Out(SYS_CON|LOG_NOTICE) << "UTP: Packet Header: " << endl;
		Out(SYS_CON|LOG_NOTICE) << "type: " << hdr.type << endl;
		Out(SYS_CON|LOG_NOTICE) << "version: " << hdr.version << endl;
		Out(SYS_CON|LOG_NOTICE) << "extension: " << hdr.extension << endl;
		Out(SYS_CON|LOG_NOTICE) << "connection_id: " << hdr.connection_id << endl;
		Out(SYS_CON|LOG_NOTICE) << "timestamp_microseconds: " << hdr.timestamp_microseconds << endl;
		Out(SYS_CON|LOG_NOTICE) << "timestamp_difference_microseconds: " << hdr.timestamp_difference_microseconds << endl;
		Out(SYS_CON|LOG_NOTICE) << "wnd_size: " << hdr.wnd_size << endl;
		Out(SYS_CON|LOG_NOTICE) << "seq_nr: " << hdr.seq_nr << endl;
		Out(SYS_CON|LOG_NOTICE) << "ack_nr: " << hdr.ack_nr << endl;
	}

	ConnectionState Connection::handlePacket(const QByteArray& packet)
	{
		QMutexLocker lock(&mutex);
		Header* hdr = 0;
		SelectiveAck* sack = 0;
		int data_off = parsePacket(packet,&hdr,&sack);
		
		DumpHeader(*hdr);
		
		updateDelayMeasurement(hdr);
		remote_wnd->packetReceived(hdr);
		ack_nr = hdr->seq_nr;
		switch (state)
		{
			case CS_SYN_SENT:
				// now we should have a state packet
				if (hdr->type == ST_STATE)
				{
					// connection estabished
					state = CS_CONNECTED;
					Out(SYS_CON|LOG_NOTICE) << "UTP: established connection with " << remote.toString() << endl;
					connected.wakeAll();
				}
				else
				{
					state = CS_CLOSED;
					data_ready.wakeAll();
				}
				break;
			case CS_IDLE:
				if (hdr->type == ST_SYN)
				{
					// Send back a state packet
					sendState();
					state = CS_CONNECTED;
					Out(SYS_CON|LOG_NOTICE) << "UTP: established connection with " << remote.toString() << endl;
				}
				else
				{
					state = CS_CLOSED;
					data_ready.wakeAll();
				}
				break;
			case CS_CONNECTED:
				if (hdr->type == ST_DATA)
				{
					// push data into local window
					int s = packet.size() - data_off;
					local_wnd->write((const bt::Uint8*)packet.data() + data_off,s);
					data_ready.wakeAll();
					
					// send back an ACK
					sendState();
				}
				else if (hdr->type == ST_STATE)
				{
					// do nothing
				}
				else if (hdr->type == ST_FIN)
				{
					eof_seq_nr = hdr->seq_nr;
					// other side now has closed the connection
					state = CS_FINISHED; // state becomes finished
				}
				else
				{
					state = CS_CLOSED;
					data_ready.wakeAll();
				}
				break;
			case CS_FINISHED:
				if (hdr->type == ST_STATE)
				{
					// Check if we need to go to the closed state
					// We can do this if all our packets have been acked and the local window
					// has been fully read
					if (remote_wnd->allPacketsAcked() && local_wnd->currentWindow() == 0)
					{
						state = CS_CLOSED;
						data_ready.wakeAll();
					}
				}
				else // TODO: make sure we handle packet loss and out of order packets
				{
					state = CS_CLOSED;
					data_ready.wakeAll();
				}
			case CS_CLOSED:
				break;
		}
		
		return state;
	}

	void Connection::sendSYN()
	{
		seq_nr = 1;
		ack_nr = 0;
		state = CS_SYN_SENT;
		
		struct timeval tv;
		gettimeofday(&tv,NULL);
		
		QByteArray ba(sizeof(Header),0);
		Header* hdr = (Header*)ba.data();
		hdr->version = 1;
		hdr->type = ST_SYN;
		hdr->extension = 0;
		hdr->connection_id = recv_connection_id;
		hdr->timestamp_microseconds = tv.tv_usec;
		hdr->timestamp_difference_microseconds = reply_micro;
		hdr->wnd_size = local_wnd->maxWindow() - local_wnd->currentWindow();
		hdr->seq_nr = seq_nr;
		hdr->ack_nr = ack_nr;
		
		remote_wnd->addPacket(ba);
		srv->sendTo((const bt::Uint8*)ba.data(),ba.size(),remote);
	}
	
	void Connection::sendState()
	{
		struct timeval tv;
		gettimeofday(&tv,NULL);
		
		QByteArray ba(sizeof(Header),0);
		Header* hdr = (Header*)ba.data();
		hdr->version = 1;
		hdr->type = ST_STATE;
		hdr->extension = 0;
		hdr->connection_id = send_connection_id;
		hdr->timestamp_microseconds = tv.tv_usec;
		hdr->timestamp_difference_microseconds = reply_micro;
		hdr->wnd_size = local_wnd->maxWindow() - local_wnd->currentWindow();
		hdr->seq_nr = seq_nr;
		hdr->ack_nr = ack_nr;
		
		remote_wnd->addPacket(ba);
		srv->sendTo((const bt::Uint8*)ba.data(),ba.size(),remote);
	}

	void Connection::sendFIN()
	{
		struct timeval tv;
		gettimeofday(&tv,NULL);
		
		QByteArray ba(sizeof(Header),0);
		Header* hdr = (Header*)ba.data();
		hdr->version = 1;
		hdr->type = ST_FIN;
		hdr->extension = 0;
		hdr->connection_id = send_connection_id;
		hdr->timestamp_microseconds = tv.tv_usec;
		hdr->timestamp_difference_microseconds = reply_micro;
		hdr->wnd_size = local_wnd->maxWindow() - local_wnd->currentWindow();
		hdr->seq_nr = seq_nr;
		hdr->ack_nr = ack_nr;
		
		remote_wnd->addPacket(ba);
		srv->sendTo((const bt::Uint8*)ba.data(),ba.size(),remote);
	}

	void Connection::sendReset()
	{
		struct timeval tv;
		gettimeofday(&tv,NULL);
		
		QByteArray ba(sizeof(Header),0);
		Header* hdr = (Header*)ba.data();
		hdr->version = 1;
		hdr->type = ST_RESET;
		hdr->extension = 0;
		hdr->connection_id = send_connection_id;
		hdr->timestamp_microseconds = tv.tv_usec;
		hdr->timestamp_difference_microseconds = reply_micro;
		hdr->wnd_size = local_wnd->maxWindow() - local_wnd->currentWindow();
		hdr->seq_nr = seq_nr;
		hdr->ack_nr = ack_nr;
		
		srv->sendTo(ba,remote);
	}

	void Connection::waitForSYN()
	{
		state = CS_IDLE;
		seq_nr = 1;
		ack_nr = 0;
	}

	void Connection::updateDelayMeasurement(const utp::Header* hdr)
	{
		struct timeval tv;
		gettimeofday(&tv,NULL);
		reply_micro = qAbs((bt::Int64)tv.tv_usec - hdr->timestamp_microseconds);
	}
		
	int Connection::parsePacket(const QByteArray& packet, Header** hdr, SelectiveAck** selective_ack)
	{
		*hdr = (Header*)packet.data();
		*selective_ack = 0;
		int data_off = sizeof(Header);
		if ((*hdr)->extension == 0)
			return data_off;
		
		// go over all header extensions to increase the data offset and watch out for selective acks
		int ext_id = (*hdr)->extension;
		UnknownExtension* ptr = 0;
		while (data_off < packet.size() && ptr->extension != 0)
		{
			ptr = (UnknownExtension*)packet.data() + data_off;
			if (ext_id == 1)
				*selective_ack = (SelectiveAck*)ptr;
				
			data_off += 2 + ptr->length;
			ext_id = ptr->extension;
		}
		
		return data_off;
	}

	bool Connection::send(const QByteArray & packet)
	{
		QMutexLocker lock(&mutex);
		if (state != CS_CONNECTED || !remote_wnd->allowedToSend(packet.size()))
			return false;
		
		struct timeval tv;
		gettimeofday(&tv,NULL);
		
		QByteArray ba(sizeof(Header) + packet.size(),0);
		Header* hdr = (Header*)ba.data();
		hdr->version = 1;
		hdr->type = ST_DATA;
		hdr->extension = 0;
		hdr->connection_id = send_connection_id;
		hdr->timestamp_microseconds = tv.tv_usec;
		hdr->timestamp_difference_microseconds = reply_micro;
		hdr->wnd_size = local_wnd->maxWindow() - local_wnd->currentWindow();
		hdr->seq_nr = seq_nr + 1;
		hdr->ack_nr = ack_nr;
		
		memcpy(ba.data() + sizeof(Header),packet.data(),packet.size());
		if (!srv->sendTo(ba,remote))
			return false;
		
		seq_nr++;
		remote_wnd->addPacket(packet);
		return true;
	}
	
	Uint32 Connection::send(const bt::Uint8* data, Uint32 len)
	{
		QMutexLocker lock(&mutex);
		if (state != CS_CONNECTED)
			return 0;
		
		bt::Uint32 space = remote_wnd->availableSpace();
		if (space == 0)
			return 0;
		
		bt::Uint32 to_send = space <= len ? space : len;
		struct timeval tv;
		gettimeofday(&tv,NULL);
		
		QByteArray ba(sizeof(Header) + to_send,0);
		Header* hdr = (Header*)ba.data();
		hdr->version = 1;
		hdr->type = ST_DATA;
		hdr->extension = 0;
		hdr->connection_id = send_connection_id;
		hdr->timestamp_microseconds = tv.tv_usec;
		hdr->timestamp_difference_microseconds = reply_micro;
		hdr->wnd_size = local_wnd->maxWindow() - local_wnd->currentWindow();
		hdr->seq_nr = seq_nr + 1;
		hdr->ack_nr = ack_nr;
		
		memcpy(ba.data() + sizeof(Header),data,to_send);
		if (!srv->sendTo(ba,remote))
			return 0;
		
		seq_nr++;
		remote_wnd->addPacket(QByteArray((const char*)data,to_send));
		return to_send;
	}

	bt::Uint32 Connection::bytesAvailable() const
	{
		QMutexLocker lock(&mutex);
		return local_wnd->currentWindow();
	}

	Uint32 Connection::recv(Uint8* buf, Uint32 max_len)
	{
		QMutexLocker lock(&mutex);
		return local_wnd->read(buf,max_len);
	}

	
	bool Connection::waitUntilConnected()
	{
		mutex.lock();
		connected.wait(&mutex);
		bool ret = state == CS_CONNECTED;
		mutex.unlock();
		return ret;
	}


	bool Connection::waitForData()
	{
		mutex.lock();
		data_ready.wait(&mutex);
		bool ret = local_wnd->currentWindow() > 0;
		mutex.unlock();
		return ret;
	}


	void Connection::close()
	{
		QMutexLocker lock(&mutex);
		if (state == CS_CONNECTED)
		{
		}
	}

}

