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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include "utpex.h"
#include <net/address.h>
#include <util/functions.h>
#include <util/log.h>
#include "peer.h"
#include "packetwriter.h"
#include <bcodec/bdecoder.h>
#include <bcodec/bencoder.h>
#include <bcodec/bnode.h>
#include "peermanager.h"


namespace bt
{
	
	bool UTPex::pex_enabled = true;

	UTPex::UTPex(Peer* peer,Uint32 id) : PeerProtocolExtension(id,peer),last_updated(0)
	{}


	UTPex::~UTPex()
	{}
	
	void UTPex::handlePacket(const Uint8* packet,Uint32 size)
	{
		if (size <= 2 || packet[1] != 1)
			return;
		
		QByteArray tmp = QByteArray::fromRawData((const char*)packet,size);
		BNode* node = 0;
		try
		{
			BDecoder dec(tmp,false,2);
			node = dec.decode();
			if (node && node->getType() == BNode::DICT)
			{
				BDictNode* dict = (BDictNode*)node;
				
				// ut_pex packet, emit signal to notify PeerManager
				BValueNode* val = dict->getValue("added");
				if (val)
				{
					QByteArray data = val->data().toByteArray();
					peer->emitPex(data);
				}
			}
		}
		catch (...)
		{
			// just ignore invalid packets
			Out(SYS_CON|LOG_DEBUG) << "Invalid ut_pex packet" << endl;
		}
		delete node;
	}
		
	bool UTPex::needsUpdate() const
	{
		return bt::CurrentTime() - last_updated >= 60*1000;
	}
	
	void UTPex::update()
	{
		PeerManager* pman = peer->getPeerManager();
		last_updated = bt::CurrentTime();
		
		std::map<Uint32,net::Address> added;
		std::map<Uint32,Uint8> flags;
		std::map<Uint32,net::Address> npeers;
		
		PeerManager::CItr itr = pman->beginPeerList();
		while (itr != pman->endPeerList())
		{
			const Peer* p = *itr;
			if (p != peer)
			{
				npeers.insert(std::make_pair(p->getID(),p->getAddress()));
				if (peers.count(p->getID()) == 0)
				{
					// new one, add to added
					added.insert(std::make_pair(p->getID(),p->getAddress()));
					
					if (p->getAddress().ipVersion() == 4)
					{
						Uint8 flag = 0;
						if (p->isSeeder())
							flag |= 0x02;
						if (p->getStats().fast_extensions)
							flag |= 0x01;
						flags.insert(std::make_pair(p->getID(),flag));
					}
				}
				else
				{
					// erase from old list, so only the dropped ones are left
					peers.erase(p->getID());
				}
			}
			itr++;
		}
		
		if (!(peers.size() == 0 && added.size() == 0))
		{
			// encode the whole lot
			QByteArray data;
			BEncoder enc(new BEncoderBufferOutput(data));
			enc.beginDict();
			enc.write(QString("added"));
			encode(enc,added);
			enc.write(QString("added.f")); 
			if (added.size() == 0)
			{
				enc.write(QString(""));
			}
			else
			{
				encodeFlags(enc,flags);
			}
			enc.write(QString("dropped"));
			encode(enc,peers);
			enc.end();
			
			peer->getPacketWriter().sendExtProtMsg(id,data);
		}

		peers = npeers;
	}	
	
	void UTPex::encode(BEncoder & enc,const std::map<Uint32,net::Address> & ps)
	{
		if (ps.size() == 0)
		{
			enc.write(QString(""));
			return;
		}
		
		Uint8* buf = new Uint8[ps.size() * 6];
		Uint32 size = 0;
		
		std::map<Uint32,net::Address>::const_iterator i = ps.begin();
		while (i != ps.end())
		{
			const net::Address & addr = i->second;
			if (addr.ipVersion() == 4)
			{
				// IPv4 addr is already in network byte order
				quint32 ip = addr.ipAddress().IPv4Addr();
				memcpy(buf + size,&ip,4);
				WriteUint16(buf,size + 4,addr.port());
				size += 6;
			}
			i++;
		}
		
		enc.write(buf,size);
		delete [] buf;
	}
	
	void UTPex::encodeFlags(BEncoder & enc,const std::map<Uint32,Uint8> & flags)
	{
		if (flags.size() == 0)
		{
			enc.write(QString(""));
			return;
		}
		
		Uint8* buf = new Uint8[flags.size()];
		Uint32 idx = 0;
		
		std::map<Uint32,Uint8>::const_iterator i = flags.begin();
		while (i != flags.end())
		{
			buf[idx++] = i->second;
			i++;
		}
		
		enc.write(buf,flags.size());
		delete [] buf;
	}
}
