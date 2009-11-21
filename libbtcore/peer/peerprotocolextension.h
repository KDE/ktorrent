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

#ifndef BT_PEERPROTOCOLEXTENSION_H
#define BT_PEERPROTOCOLEXTENSION_H

#include <btcore_export.h>
#include <util/constants.h>

namespace bt
{
	class Peer;

	const Uint32 UT_PEX_ID = 1;
	const Uint32 UT_METADATA_ID = 2;
	
	/**
		Base class for protocol extensions
	*/
	class BTCORE_EXPORT PeerProtocolExtension
	{
	public:
		PeerProtocolExtension(bt::Uint32 id,Peer* peer);
		virtual ~PeerProtocolExtension();
		
		/// Virtual update function does nothing, needs to be overriden if update
		virtual void update();
		
		/// Does this needs to be update
		virtual bool needsUpdate() const {return false;}
		
		/// Handle a packet
		virtual void handlePacket(const bt::Uint8* packet, Uint32 size) = 0;
		
		/// Send an extension protocol packet 
		void sendPacket(const QByteArray & data);
		
		/// Change the ID
		void changeID(Uint32 id);
		
	protected:
		bt::Uint32 id;
		Peer* peer;
	};

}

#endif // BT_PEERPROTOCOLEXTENSION_H
