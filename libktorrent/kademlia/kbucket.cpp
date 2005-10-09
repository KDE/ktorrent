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
#include "kbucket.h"

namespace dht
{
	KBucketEntry::KBucketEntry() : ip_address(0),udp_port(0)
	{
	}
	
	KBucketEntry::KBucketEntry(Uint32 ip,Uint16 port,const Key & id)
		: ip_address(ip),udp_port(port),node_id(id)
	{
	}
		
	KBucketEntry::KBucketEntry(const KBucketEntry & other)
	: ip_address(other.ip_address),udp_port(other.udp_port),node_id(other.node_id)
	{}

		
	KBucketEntry::~KBucketEntry()
	{}

	KBucketEntry & KBucketEntry::operator = (const KBucketEntry & other)
	{
		ip_address = other.ip_address;
		udp_port = other.udp_port;
		node_id = other.node_id;
		return *this;
	}

	KBucket::KBucket()
	{}
	
	
	KBucket::~KBucket()
	{}

}

