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
#include <net/address.h>
#include <util/functions.h>
#include <util/log.h>
#include "utpex.h"
#include "peer.h"
#include "packetwriter.h"
#include "bdecoder.h"
#include "bencoder.h"
#include "bnode.h"
#include "peermanager.h"


namespace bt
{

	UTPex::UTPex(Peer* peer,Uint32 id) : peer(peer),id(id),last_updated(0)
	{}


	UTPex::~UTPex()
	{}
	
	
	
	void UTPex::handlePexPacket(const Uint8* packet,Uint32 size)
	{
		if (size <= 2 || packet[1] != 1)
			return;
		
		QByteArray tmp;
		tmp.setRawData((const char*)packet,size);
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
			Out(SYS_CON|LOG_DEBUG) << "Invalid extended packet" << endl;
		}
		delete node;
		tmp.resetRawData((const char*)packet,size);
	}
		
	bool UTPex::needsUpdate() const
	{
		return bt::GetCurrentTime() - last_updated >= 60*1000;
	}
	
	void UTPex::update(PeerManager* pman)
	{
		last_updated = bt::GetCurrentTime();
		
		std::map<Uint32,net::Address> added;
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
			enc.write("added");
			encode(enc,added);
			enc.write("added.f"); // no idea what this added.f thing means
			enc.write("");
			enc.write("dropped");
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
			enc.write("");
			return;
		}
		
		Uint8* buf = new Uint8[ps.size() * 6];
		Uint32 size = 0;
		
		std::map<Uint32,net::Address>::const_iterator i = ps.begin();
		while (i != ps.end())
		{
			const net::Address & addr = i->second;
			WriteUint32(buf,size,addr.ip());
			WriteUint16(buf,size + 4,addr.port());
			size += 6;
			i++;
		}
		
		enc.write(buf,size);
		delete [] buf;
	}
}
