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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include "packetwriter.h"
#include "peer.h"
#include "request.h"
#include "chunk.h"
#include <util/bitset.h>
#include "packet.h"
#include "uploadcap.h"
#include <util/log.h>
#include "globals.h"

namespace bt
{
	

	PacketWriter::PacketWriter(Peer* peer) : peer(peer)
	{
		uploaded = 0;
		packets.setAutoDelete(true);
	}


	PacketWriter::~PacketWriter()
	{
		UploadCap::instance().killed(this);
	}

	void PacketWriter::sendPacket(const Packet & p)
	{
		peer->sendData(p.getHeader(),p.getHeaderLength());
		if (p.getDataLength() > 0)
			peer->sendData(p.getData(),p.getDataLength());
	}

	void PacketWriter::sendChoke()
	{
		if (peer->am_choked == true)
			return;
		
		if (packets.count() == 0)
			sendPacket(Packet(CHOKE));
		else
			packets.append(new Packet(CHOKE));

		peer->am_choked = true;
	}
	
	void PacketWriter::sendUnchoke()
	{
		if (peer->am_choked == false)
			return;
		
		if (packets.count() == 0)
			sendPacket(Packet(UNCHOKE));
		else
			packets.append(new Packet(UNCHOKE));
		peer->am_choked = false;
	}
	
	void PacketWriter::sendInterested()
	{
		if (peer->am_interested == true)
			return;
		
		if (packets.count() == 0)
			sendPacket(Packet(INTERESTED));
		else
			packets.append(new Packet(INTERESTED));
		peer->am_interested = true;
	}
	
	void PacketWriter::sendNotInterested()
	{
		if (peer->am_interested == false)
			return;
		
		if (packets.count() == 0)
			sendPacket(Packet(NOT_INTERESTED));
		else
			packets.append(new Packet(NOT_INTERESTED));
		peer->am_interested = false;
	}

	/*
	void PacketWriter::sendKeepAlive()
	{
		Uint8 buf[4];
		WriteUint32(buf,0,0);
		peer->sendData(buf,4);
	}*/
	
	void PacketWriter::sendRequest(const Request & r)
	{
		if (packets.count() == 0)
			sendPacket(Packet(r,false));
		else
			packets.append(new Packet(r,false));
	}
	
	void PacketWriter::sendCancel(const Request & r)
	{
		if (packets.count() == 0)
			sendPacket(Packet(r,true));
		else
			packets.append(new Packet(r,true));
	}
	
	void PacketWriter::sendHave(Uint32 index)
	{
		if (packets.count() == 0)
			sendPacket(Packet(index));
		else
			packets.append(new Packet(index));
	}
	
	void PacketWriter::sendBitSet(const BitSet & bs)
	{
		if (packets.count() == 0)
			sendPacket(Packet(bs));
		else
			packets.append(new Packet(bs));
	}
			
	void PacketWriter::sendChunk(Uint32 index,Uint32 begin,Uint32 len,const Chunk & ch)
	{
		if (begin >= ch.getSize() || begin + len > ch.getSize())
		{
			Out() << "Warning : Illegal piece request" << endl;
			Out() << "\tChunk : index " << index << " size = " << ch.getSize() << endl;
			Out() << "\tPiece : begin = " << begin << " len = " << len << endl;
		}
		else
		{
			// try to send it, if we can add it to the queue
			if (UploadCap::instance().allow(this,len))
				sendPacket(Packet(index,begin,len,ch));
			else
				packets.append(new Packet(index,begin,len,ch));
		}
	}

	Uint32 PacketWriter::update()
	{
		Uint32 data_sent = uploaded;
		uploaded = 0;

		if (packets.count() == 0)
			return data_sent;
		
		while (packets.count() > 0)
		{
			Packet* p = packets.first();
			// send packets with no payload immediatly
			if (p->getType() != PIECE)
			{
				sendPacket(*p);
				packets.removeFirst();
			}
			else
			{
				break;
			}
		}

		return data_sent;
	}

	Uint32 PacketWriter::uploadUnsentBytes(Uint32 num_bytes)
	{
		if (packets.count() == 0)
			return 0;
		
		Packet* p = packets.first();	
		Uint32 bytes_written = 0;
		Uint32 written = p->getDataWritten();
		if (written == 0)
		{
			// we haven't written anything yet
			// so send header
			peer->sendData(p->getHeader(),p->getHeaderLength());
			p->dataWritten(p->getHeaderLength());
			// now try to send data
			if (num_bytes == p->getDataLength())
			{
				// we can send the packet fully
				peer->sendData(p->getData(),p->getDataLength(),true);
				p->dataWritten(p->getDataLength());
				bytes_written += p->getDataLength();
				// remove the packet
				packets.removeFirst();
			}
			else if (num_bytes > 0)
			{
				// we can send some data
				peer->sendData(p->getData(),num_bytes,true);
				p->dataWritten(num_bytes);
				bytes_written += num_bytes;
			}
		}
		else
		{
			// calculate how many bytes are left
			Uint32 dl = p->getDataLength();
			Uint32 written = p->getDataWritten() - p->getHeaderLength();
			Uint32 left = dl - written;
			// try to send the remaining bytes
			if (num_bytes == left)
			{
				// we can send the remaining bytes
				peer->sendData(p->getData() + written,left,true);
				p->dataWritten(left);
				bytes_written += left;
				// remove the packet
				packets.removeFirst();
			}
			else if (num_bytes > 0)
			{
				// send another part of the data
				peer->sendData(p->getData() + written,num_bytes,true);
				p->dataWritten(num_bytes);
				bytes_written += num_bytes;
			}
		}

		uploaded += bytes_written;
		return bytes_written;
	}
}
