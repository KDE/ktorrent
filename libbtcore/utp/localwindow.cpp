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

#include "localwindow.h"
#include "utpprotocol.h"
#include <QtAlgorithms>
#include <util/log.h>

using namespace bt;

namespace utp
{

	FuturePacket::FuturePacket(bt::Uint16 seq_nr, const bt::Uint8* data, bt::Uint32 size) 
		: seq_nr(seq_nr),data((const char*)data,size)
	{
	}

	FuturePacket::~FuturePacket()
	{
	}
	
	LocalWindow::LocalWindow(bt::Uint32 cap) : bt::CircularBuffer(cap),window_space(cap)
	{
		
	}

	LocalWindow::~LocalWindow()
	{
		qDeleteAll(future_packets);
	}
	
	
	void LocalWindow::setLastSeqNr(bt::Uint16 lsn)
	{
		last_seq_nr = lsn;
	}

	bt::Uint32 LocalWindow::read(bt::Uint8* data, bt::Uint32 max_len)
	{
		bt::Uint32 ret = CircularBuffer::read(data, max_len);
		window_space += ret;
		return ret;
	}


	void LocalWindow::checkFuturePackets()
	{
		QLinkedList<FuturePacket*>::iterator itr = future_packets.begin();
		while (itr != future_packets.end())
		{
			FuturePacket* pkt = *itr;
			if (pkt->seq_nr == last_seq_nr + 1)
			{
				last_seq_nr = pkt->seq_nr;
				if (write((const bt::Uint8*)pkt->data.data(),pkt->data.size()) != pkt->data.size())
					Out(SYS_UTP|LOG_DEBUG) << "LocalWindow::packetReceived write failed " << endl;
				delete pkt;
				itr = future_packets.erase(itr);
			}
			else
				break;
		}
	}

	
	bool LocalWindow::packetReceived(const utp::Header* hdr,const bt::Uint8* data,bt::Uint32 size)
	{
		// Drop duplicate data packets
		if (hdr->seq_nr <= last_seq_nr) 
			return true;
		
		if (hdr->seq_nr != last_seq_nr + 1)
		{
			// insert the packet into the future_packets list
			QLinkedList<FuturePacket*>::iterator itr = future_packets.begin();
			while (itr != future_packets.end())
			{
				FuturePacket* pkt = *itr;
				if (pkt->seq_nr < hdr->seq_nr)
				{
					itr++;
				}
				else if (pkt->seq_nr == hdr->seq_nr)
				{
					// Dupe, just return
					return true;
				}
				else
				{
					// we have found a packet with a higher sequence number
					// so insert
					future_packets.insert(itr,new FuturePacket(hdr->seq_nr,data,size));
					break;
				}
			}
			
			// at the end and not inserted yet, so just append
			if (itr == future_packets.end())
				future_packets.append(new FuturePacket(hdr->seq_nr,data,size));
			
			window_space -= size;
			checkFuturePackets();
		}
		else
		{
			if (availableSpace() < size)
			{
				Out(SYS_UTP|LOG_DEBUG) << "Not enough space in local window " << availableSpace() << " " << size << endl;
				return false;
			}
			
			last_seq_nr = hdr->seq_nr;
			if (write(data,size) != size)
				Out(SYS_UTP|LOG_DEBUG) << "LocalWindow::packetReceived write failed " << endl;
			window_space -= size;
			checkFuturePackets();
		}
		
		return true;
	}

	
	bt::Uint32 LocalWindow::selectiveAckBits() const
	{
		if (future_packets.isEmpty())
			return 0;
		else
			return future_packets.last()->seq_nr - last_seq_nr - 1;
	}


	void LocalWindow::fillSelectiveAck(SelectiveAck* sack)
	{
		// First turn off all bits
		memset(sack->bitmask,0,sack->length);
		
		QLinkedList<FuturePacket*>::iterator itr = future_packets.begin();
		while (itr != future_packets.end())
		{
			Ack(sack,(*itr)->seq_nr - last_seq_nr);
			itr++;
		}
	}

}

