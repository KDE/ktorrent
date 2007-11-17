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
#include <util/error.h>
#include <util/functions.h>
#include "pack.h"

using namespace bt;
using namespace KNetwork;

namespace dht
{

	void PackBucketEntry(const KBucketEntry & e,QByteArray & ba,Uint32 off)
	{
		// first check size
		if (off + 26 > ba.size())
			throw bt::Error("Not enough room in buffer");
		
		Uint8* data = (Uint8*)ba.data();
		Uint8* ptr = data + off;
		
		const KInetSocketAddress & addr = e.getAddress();
		// copy ID, IP address and port into the buffer
		memcpy(ptr,e.getID().getData(),20);
		bt::WriteUint32(ptr,20,addr.ipAddress().IPv4Addr());
		bt::WriteUint16(ptr,24,addr.port());
	}
	
	KBucketEntry UnpackBucketEntry(const QByteArray & ba,Uint32 off)
	{
		if (off + 26 > ba.size())
			throw bt::Error("Not enough room in buffer");
		
		const Uint8* data = (Uint8*)ba.data();
		const Uint8* ptr = data + off;
		
		// get the port, ip and key);
		Uint16 port = bt::ReadUint16(ptr,24);
		Uint8 key[20];
		memcpy(key,ptr,20);
		
		return KBucketEntry(KInetSocketAddress(KIpAddress(ptr+20,4),port),dht::Key(key));
	}

}
