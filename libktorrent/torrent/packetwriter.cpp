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
#include "bitset.h"
#include "packet.h"
#include "uploadcap.h"
#include <util/log.h>
#include "globals.h"

namespace bt
{
	

	PacketWriter::PacketWriter(Peer* peer) : peer(peer)
	{
		packets.setAutoDelete(true);
	}


	PacketWriter::~PacketWriter()
	{}

	void PacketWriter::sendPacket(const Packet & p)
	{
		peer->sendData(p.getHeader(),p.getHeaderLength());
		if (p.getDataLength() > 0)
			peer->sendData(p.getData(),p.getDataLength());
	}

	void PacketWriter::sendChoke()
	{
		if (packets.count() == 0)
			sendPacket(Packet(CHOKE));
		else
			packets.append(new Packet(CHOKE));

		peer->am_choked = true;
	}
	
	void PacketWriter::sendUnchoke()
	{
		if (packets.count() == 0)
			sendPacket(Packet(UNCHOKE));
		else
			packets.append(new Packet(UNCHOKE));
		peer->am_choked = false;
	}
	
	void PacketWriter::sendInterested()
	{
		if (packets.count() == 0)
			sendPacket(Packet(INTERESTED));
		else
			packets.append(new Packet(INTERESTED));
		peer->am_interested = true;
	}
	
	void PacketWriter::sendNotInterested()
	{
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
			packets.append(new Packet(index,begin,len,ch));
		}
	}

	bool PacketWriter::sendBigPacket(Packet & p,Uint32 & bytes_written)
	{
		bytes_written = 0;
		Uint32 written = p.getDataWritten();
		if (written == 0)
		{
			// we haven't written anything yet
			// so send header
			peer->sendData(p.getHeader(),p.getHeaderLength());
			p.dataWritten(p.getHeaderLength());
			// now try to send data
			Uint32 allowed = UploadCap::allow(p.getDataLength());
			if (allowed == p.getDataLength())
			{
				// we can send the packet fully
				peer->sendData(p.getData(),p.getDataLength(),true);
				p.dataWritten(p.getDataLength());
				bytes_written += p.getDataLength();
				return true;
			}
			else if (allowed > 0)
			{
				// we can send some data
				peer->sendData(p.getData(),allowed,true);
				p.dataWritten(allowed);
				bytes_written += allowed;
				return false;
			}
		}
		else
		{
			// calculate how many bytes are left
			Uint32 dl = p.getDataLength();
			Uint32 written = p.getDataWritten() - p.getHeaderLength();
			Uint32 left = dl - written;
			// try to send the remaining bytes
			Uint32 allowed = UploadCap::allow(left);
			if (allowed == left)
			{
				// we can send the remaining bytes
				peer->sendData(p.getData() + written,left,true);
				p.dataWritten(left);
				bytes_written += left;
				return true;
			}
			else if (allowed > 0)
			{
				// send another part of the data
				peer->sendData(p.getData() + written,allowed,true);
				p.dataWritten(allowed);
				bytes_written += allowed;
				return false;
			}
		}

		return false;
	}

	Uint32 PacketWriter::update()
	{
		if (packets.count() == 0)
			return 0;

		Uint32 data_sent = 0;
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
				if (sendBigPacket(*p,data_sent))
				{
					// packet was fully sent
					// get rid of it
					packets.removeFirst();
				}
				else
				{
					// we couldn't send the entire packet
					// so break out of loop
					return data_sent;
				}
			}
		}

		return data_sent;
	}
}
