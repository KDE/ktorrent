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

#include "remotewindow.h"
#include "utpprotocol.h"
#include "connection.h"
#include <util/log.h>
#include <util/functions.h>

using namespace bt;

namespace utp
{
	
	
	UnackedPacket::UnackedPacket(const QByteArray& data, bt::Uint16 seq_nr, bt::TimeStamp send_time) 
		: data(data),seq_nr(seq_nr),send_time(send_time)
	{
	}

	UnackedPacket::~UnackedPacket()
	{
	}

	
	RemoteWindow::RemoteWindow() : cur_window(0),max_window(64 * 1024),wnd_size(0),last_ack_nr(0),last_ack_receive_count(0)
	{

	}

	RemoteWindow::~RemoteWindow()
	{
		qDeleteAll(unacked_packets);
	}

	void RemoteWindow::packetReceived(const utp::Header* hdr,const SelectiveAck* sack,Retransmitter* conn)
	{
		if (hdr->ack_nr == last_ack_nr)
		{
			if (hdr->type == ST_STATE)
				last_ack_receive_count++;
		}
		else
		{
			last_ack_nr = hdr->ack_nr;
			last_ack_receive_count = 1;
		}
		
		wnd_size = hdr->wnd_size;
		
		bt::TimeStamp now = bt::Now();
		QList<UnackedPacket*>::iterator i = unacked_packets.begin();
		while (i != unacked_packets.end())
		{
			UnackedPacket* up = *i;
			if (up->seq_nr <= hdr->ack_nr)
			{
				// everything up until the ack_nr in the header is acked
				conn->updateRTT(hdr,now - up->send_time,up->data.size());
				cur_window -= up->data.size();
				delete up;
				i = unacked_packets.erase(i);
			}
			else if (sack)
			{
				if (Acked(sack,up->seq_nr - hdr->ack_nr))
				{
					conn->updateRTT(hdr,now - up->send_time,up->data.size());
					cur_window -= up->data.size();
					delete up;
					i = unacked_packets.erase(i);
				}
				else
					i++;
			}
			else
				break;
		}
		
		if (!unacked_packets.isEmpty())
			checkLostPackets(hdr,sack,conn);
	}

	void RemoteWindow::addPacket(const QByteArray& data,bt::Uint16 seq_nr,bt::TimeStamp send_time)
	{
		cur_window += data.size();
		unacked_packets.append(new UnackedPacket(data,seq_nr,send_time));
	}

	void RemoteWindow::checkLostPackets(const utp::Header* hdr, const utp::SelectiveAck* sack,Retransmitter* conn)
	{
		bool lost_packets = false;
		QList<UnackedPacket*>::iterator itr = unacked_packets.begin();
		UnackedPacket* first_unacked = *itr;
		if (last_ack_receive_count >= 3 && first_unacked->seq_nr == hdr->ack_nr + 1)
		{
			// packet has been lost
			Out(SYS_GEN|LOG_DEBUG) << "Packet with sequence number " << first_unacked->seq_nr << " lost" << endl;
			conn->retransmit(first_unacked->data,first_unacked->seq_nr);
			first_unacked->send_time = bt::Now();
			lost_packets = true;
			itr++;
		}
		
		
		while (sack && itr != unacked_packets.end())
		{
			if (lost(sack,(*itr)->seq_nr - hdr->ack_nr))
			{
				Out(SYS_GEN|LOG_DEBUG) << "Packet with sequence number " << (*itr)->seq_nr << " lost" << endl;
				conn->retransmit((*itr)->data,(*itr)->seq_nr);
				(*itr)->send_time = bt::Now();
				lost_packets = true;
			}
			itr++;
		}
		
		if (lost_packets)
			max_window = (bt::Uint32)qRound(0.78 * max_window);
	}

	bool RemoteWindow::lost(const utp::SelectiveAck* sack, bt::Uint16 seq_nr)
	{
		// A packet is lost if 3 packets have been acked after it
		bt::Uint32 acked = 0;
		for (bt::Uint16 i = seq_nr + 1;i < sack->length * 8 && acked < 3;i++)
		{
			if (Acked(sack,i))
				acked++;
		}
		
		return acked >= 3;
	}

	void RemoteWindow::timeout(Retransmitter* conn)
	{
		max_window = MIN_PACKET_SIZE;
		bt::TimeStamp now = bt::Now();
		// When a timeout occurs retransmit packets which are lost longer then 
		// the max timeout
		foreach (UnackedPacket* pkt,unacked_packets)
		{
			if (now - pkt->send_time > conn->currentTimeout())
			{
				conn->retransmit(pkt->data,pkt->seq_nr);
				pkt->send_time = bt::Now();
				Out(SYS_GEN|LOG_DEBUG) << "Packet with sequence number " << pkt->seq_nr << " lost" << endl;
			}
		}
	}

	void RemoteWindow::updateWindowSize(double scaled_gain)
	{
		int d = (int)qRound(scaled_gain);
		if ((int)max_window + d < 0)
			max_window = 0;
		else
			max_window += d;
	//	Out(SYS_GEN|LOG_DEBUG) << "RemoteWindow::updateWindowSize " << scaled_gain << " " << max_window << endl;
	}

	void RemoteWindow::clear()
	{
		qDeleteAll(unacked_packets);
	}

}

