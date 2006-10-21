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
//#define LOG_PACKET

#include <util/log.h>
#include <util/file.h>
#include <util/functions.h>
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


	PacketWriter::PacketWriter(Peer* peer) : peer(peer),mutex(true) // this is a recursive mutex
	{
		uploaded = 0;
		uploaded_non_data = 0;
		data_packets.setAutoDelete(true);
		control_packets.setAutoDelete(true);
		curr_packet = 0;
		ctrl_packets_sent = 0;
	}


	PacketWriter::~PacketWriter()
	{
	}
	
	void PacketWriter::queuePacket(Packet* p)
	{
		mutex.lock();
		if (p->getType() == PIECE)
			data_packets.append(p);
		else
			control_packets.append(p);
		mutex.unlock();
	}
	
	

	void PacketWriter::sendChoke()
	{
		if (peer->am_choked == true)
			return;
		
		queuePacket(new Packet(CHOKE));
		peer->am_choked = true;
		peer->stats.has_upload_slot = false;
	}
	
	void PacketWriter::sendUnchoke()
	{
//		Out() << "UNCHOKE" << endl;
		if (peer->am_choked == false)
			return;
		
		queuePacket(new Packet(UNCHOKE));
		peer->am_choked = false;
		peer->stats.has_upload_slot = true;
	}
	
	void PacketWriter::sendEvilUnchoke()
	{
		queuePacket(new Packet(UNCHOKE));
		peer->am_choked = true;
		peer->stats.has_upload_slot = false;
	}
	
	void PacketWriter::sendInterested()
	{
		if (peer->am_interested == true)
			return;
		
		queuePacket(new Packet(INTERESTED));
		peer->am_interested = true;
	}
	
	void PacketWriter::sendNotInterested()
	{
		if (peer->am_interested == false)
			return;
		
		queuePacket(new Packet(NOT_INTERESTED));
		peer->am_interested = false;
	}
	
	void PacketWriter::sendRequest(const Request & r)
	{
		queuePacket(new Packet(r,bt::REQUEST));
	}
	
	void PacketWriter::sendCancel(const Request & r)
	{
		queuePacket(new Packet(r,bt::CANCEL));
	}
	
	void PacketWriter::sendReject(const Request & r)
	{
		queuePacket(new Packet(r,bt::REJECT_REQUEST));
	}
	
	void PacketWriter::sendHave(Uint32 index)
	{
		queuePacket(new Packet(index,bt::HAVE));
	}
	
	void PacketWriter::sendPort(Uint16 port)
	{
		queuePacket(new Packet(port));
	}
	
	void PacketWriter::sendBitSet(const BitSet & bs)
	{
		queuePacket(new Packet(bs));
	}
	
	void PacketWriter::sendHaveAll()
	{
		queuePacket(new Packet(bt::HAVE_ALL));
	}

	void PacketWriter::sendHaveNone()
	{
		queuePacket(new Packet(bt::HAVE_NONE));
	}
	
	void PacketWriter::sendSuggestPiece(Uint32 index)
	{
		queuePacket(new Packet(index,bt::SUGGEST_PIECE));
	}
	
	void PacketWriter::sendAllowedFast(Uint32 index)
	{
		queuePacket(new Packet(index,bt::ALLOWED_FAST));
	}
			
	bool PacketWriter::sendChunk(Uint32 index,Uint32 begin,Uint32 len,Chunk * ch)
	{
//		Out() << "sendChunk " << index << " " << begin << " " << len << endl;
		if (begin >= ch->getSize() || begin + len > ch->getSize())
		{
			Out(SYS_CON|LOG_NOTICE) << "Warning : Illegal piece request" << endl;
			Out(SYS_CON|LOG_NOTICE) << "\tChunk : index " << index << " size = " << ch->getSize() << endl;
			Out(SYS_CON|LOG_NOTICE) << "\tPiece : begin = " << begin << " len = " << len << endl;
			return false;
		}
		else if (!ch || ch->getData() == 0)
		{
			Out(SYS_CON|LOG_NOTICE) << "Warning : attempted to upload an invalid chunk" << endl;
			return false;
		}
		else
		{
	/*		Out(SYS_CON|LOG_DEBUG) << QString("Uploading %1 %2 %3 %4 %5")
					.arg(index).arg(begin).arg(len).arg((Q_ULLONG)ch,0,16).arg((Q_ULLONG)ch->getData(),0,16) 
					<< endl;;
	*/
			queuePacket(new Packet(index,begin,len,ch));
			return true;
		}
	}
	
	Packet* PacketWriter::selectPacket()
	{
		// this function should ensure that between
		// each data packet at least 3 control packets are sent
		// so requests can get through
		if (ctrl_packets_sent < 3)
		{
			// try to send another control packet
			if (control_packets.count() > 0)
				return control_packets.first();
			else if (data_packets.count() > 0)
				return data_packets.first(); 
		}
		else
		{
			if (data_packets.count() > 0)
			{
				ctrl_packets_sent = 0;
				return data_packets.first();
			}
			else if (control_packets.count() > 0)
				return control_packets.first();
		}
		return 0;
	}
	
	Uint32 PacketWriter::onReadyToWrite(Uint8* data,Uint32 max_to_write)
	{
		mutex.lock();
		if (!curr_packet)
			curr_packet = selectPacket();
		Uint32 written = 0;
		while (curr_packet && written < max_to_write)
		{
			Packet* p = curr_packet;
			bool count_as_data = false;
			Uint32 ret = p->putInOutputBuffer(data + written,max_to_write - written,count_as_data);
			written += ret;
			if (count_as_data)
				uploaded += ret;
			else
				uploaded_non_data += ret;
					
			if (p->isSent())
			{
				// packet sent, so remove it
				if (p->getType() == PIECE)
				{
					// remove data packet
					data_packets.removeFirst();
					// reset ctrl_packets_sent so the next packet should be a ctrl packet
					ctrl_packets_sent = 0;  
					curr_packet = selectPacket();
				}
				else
				{
					// remove control packet and select another one to send
					control_packets.removeFirst();
					ctrl_packets_sent++;
					curr_packet = selectPacket();
				}
			}
			else
			{
				// we can't send it fully, so break out of loop
				break;
			}
		}
		mutex.unlock();
		return written;
	}
	
	bool PacketWriter::hasBytesToWrite() const
	{
		return getNumPacketsToWrite() > 0;
	}
	
	Uint32 PacketWriter::getUploadedDataBytes() const
	{
		mutex.lock();
		Uint32 ret = uploaded;
		uploaded = 0;
		mutex.unlock();
		return ret;
	}
	
	Uint32 PacketWriter::getUploadedNonDataBytes() const
	{
		mutex.lock();
		Uint32 ret = uploaded_non_data;
		uploaded_non_data = 0;
		mutex.unlock();
		return ret;
	}
	
	Uint32 PacketWriter::getNumPacketsToWrite() const
	{
		mutex.lock();
		Uint32 ret = data_packets.count() + control_packets.count();
		mutex.unlock();
		return ret;
	}
	
	Uint32 PacketWriter::getNumDataPacketsToWrite() const
	{
		mutex.lock();
		Uint32 ret = data_packets.count();
		mutex.unlock();
		return ret;
	}
	
	void PacketWriter::doNotSendPiece(const Request & req,bool reject)
	{
		QMutexLocker locker(&mutex);
		Packet* p = data_packets.first();
		while (p)
		{
			if (p->isPiece(req) && !p->sending())
			{
				// remove current item
				data_packets.remove();
				p = data_packets.current();
				if (reject)
				{
					// queue a reject packet
					sendReject(req);
				}
			}
			else
			{
				p = data_packets.next();
			}
		}
	}
	
	void PacketWriter::clearPieces()
	{
		QMutexLocker locker(&mutex);
		Packet* p = data_packets.first();
		while (p)
		{
			if (p->getType() == bt::PIECE && !p->sending())
			{
				// remove current item
				data_packets.remove();
				p = data_packets.current();
			}
			else
			{
				p = data_packets.next();
			}
		}
	}
}
