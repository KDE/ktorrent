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
#ifndef BTUTPEX_H
#define BTUTPEX_H
		
#include <map>
#include <net/address.h>
#include <util/constants.h>

namespace bt
{
	class Peer;
	class PeerManager;
	class BEncoder;

	/**
	 * @author Joris Guisson <joris.guisson@gmail.com>
	 * 
	 * Class which handles µTorrent's peer exchange
	*/
	class UTPex
	{
	public:
		UTPex(Peer* peer,Uint32 id);
		virtual ~UTPex();

		/**
		 * Handle a PEX packet
		 * @param packet The packet 
		 * @param size The size of the packet
		 */
		void handlePexPacket(const Uint8* packet,Uint32 size);
		
		/// Do we need to update PEX (should happen every minute)
		bool needsUpdate() const;
		
		/// Send a new PEX packet to the Peer
		void update(PeerManager* pman);
		
		/// Change the ID used in the extended packets
		void changeID(Uint32 nid) {id = nid;}
	private:
		void encode(BEncoder & enc,const std::map<Uint32,net::Address> & ps);
		
	private:
		Peer* peer;
		Uint32 id; 
		std::map<Uint32,net::Address> peers; 
		TimeStamp last_updated;
	};

}

#endif
