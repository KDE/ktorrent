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
#include "localwindow.h"
#include "remotewindow.h"

namespace utp
{
	Connection::Connection(quint16 recv_connection_id, Type type, const net::Address& remote, UTPServer* srv) 
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
	}

	Connection::~Connection()
	{
		delete local_wnd;
		delete remote_wnd;
	}

	void Connection::handlePacket(const QByteArray& packet)
	{
		Header* hdr = 0;
		SelectiveAck* sack = 0;
		int data_off = parsePacket(packet,&hdr,&sack);
		
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
				}
				else
				{
					srv->kill(this);
				}
				break;
			case CS_IDLE:
				if (hdr->type == ST_SYN)
				{
					// Send back a state packet
					sendState();
					state = CS_CONNECTED;
				}
				else
				{
					srv->kill(this);
				}
				break;
			case CS_CONNECTED:
				if (hdr->type == ST_DATA)
				{
					// push data into local window
					int s = packet.size() - data_off;
					local_wnd->write((const bt::Uint8*)packet.data() + data_off,s);
				}
				else if (hdr->type == ST_STATE)
				{
					// do nothing
				}
				else if (hdr->type == ST_FIN)
				{
					eof_seq_nr = hdr->seq_nr;
					// other side now has closed the connection
				}
				else
				{
					srv->kill(this);
				}
				break;
		}
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
		hdr->seq_nr = seq_nr++;
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
		hdr->seq_nr = seq_nr++;
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
		hdr->seq_nr = seq_nr++;
		hdr->ack_nr = ack_nr;
		
		remote_wnd->addPacket(ba);
		srv->sendTo((const bt::Uint8*)ba.data(),ba.size(),remote);
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
		reply_micro = qAbs(tv.tv_usec - hdr->timestamp_microseconds);
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

}

