/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.           *
 ***************************************************************************/

#include "ipblocklist.h"
#include <qmap.h>
#include <qstring.h>
#include <util/constants.h>
 #include <util/log.h>
 #include "globals.h"


namespace bt
{
	Uint32 toUint32(QString& ip, bool* ok)
	{
		bool test;
		*ok = true;

		Uint32 ret = ip.section('.',0,0).toULongLong(&test);
		if(!test) *ok=false;
		ret <<= 8;
		ret |= ip.section('.',1,1).toULong(&test);
		if(!test) *ok=false;
		ret <<= 8;
		ret |= ip.section('.',2,2).toULong(&test);
		if(!test) *ok=false;
		ret <<= 8;
		ret |= ip.section('.',3,3).toULong(&test);
		if(!test) *ok=false;

		if(*ok)
		{
			// 			Out() << "IP: " << ip << " parsed: " << ret << endl;
			return ret;
		}
		else
		{
			// 			Out() << "Could not parse IP " << ip << ".  IP blocklist might not be working." << endl;
			return 0;
		}
	}

	IPBlocklist::IPBlocklist()
	{
		insert("0.0.0.0",3);
		addRange("3.*.*.*");
	}

	IPBlocklist::IPBlocklist(const IPBlocklist & ) {}

	void IPBlocklist::insert( QString ip, int state )
	{
		bool ok;
		Uint32 ipi = toUint32(ip, &ok);
		if(!ok)
			return;
		IPKey key(ipi,0xFFFFFFFF); //-- you can test ranges here. Just specify your mask.
		insertRangeIP(key, state);
		Out() << "IP " << ip << " banned." << endl;
	}

	void IPBlocklist::addRange(QString ip)
	{
		bool ok;
		int tmp = 0;
		Uint32 addr = 0;
		Uint32 mask = 0xFFFFFFFF;

		tmp = ip.section('.',0,0).toInt(&ok);
		if(!ok)
		{
			if(ip.section('.',0,0) == "*")
				mask &= 0x00FFFFFF;
			else return; //illegal character
		}
		else
			addr = tmp;

		tmp = ip.section('.',1,1).toInt(&ok);
		if(!ok)
		{
			addr <<= 8;
			if(ip.section('.',1,1) == "*")
				mask &= 0xFF00FFFF;
			else return; //illegal character
		}
		else
		{
			addr <<= 8;
			addr |= tmp;
		}

		tmp = ip.section('.',2,2).toInt(&ok);
		if(!ok)
		{
			addr <<= 8;
			if(ip.section('.',2,2) == "*")
				mask &= 0xFFFF00FF;
			else return; //illegal character
		}
		else
		{
			addr <<= 8;
			addr |= tmp;
		}

		tmp = ip.section('.',3,3).toInt(&ok);
		if(!ok)
		{
			addr <<= 8;
			if(ip.section('.',3,3) == "*")
				mask &=0xFFFFFF00;
			else return; //illegal character
		}
		else
		{
			addr <<= 8;
			addr |= tmp;
		}

		IPKey key(addr, mask);
		this->insertRangeIP(key);
	}

	void IPBlocklist::insertRangeIP(IPKey& key, int state)
	{
// 		Out() << "Blocked range: " << key.m_ip << " - " << key.m_mask << endl;
		QMap<IPKey, int>::iterator it;
		if ((it = m_peers.find(key)) != m_peers.end())
		{

			if(it.key().m_mask != key.m_mask)
			{
				int st = it.data();
				IPKey key1(key.m_ip, it.key().m_mask | key.m_mask);
				m_peers.insert(key1, state+st);
				return;
			}
			m_peers[key]+= state;
		}
		else
			m_peers.insert(key,state);
	}

	bool IPBlocklist::isBlocked( QString& ip )
	{
		bool ok;
		Uint32 ipi = toUint32(ip,&ok);
		if (!ok)
			return false;
		IPKey key(ipi);

		QMap<IPKey, int>::iterator it;
		it = m_peers.find(key);
		if (it==m_peers.end())
			return false;

		return m_peers[key] >= 3;
	}

	/***  IPKey   *****************************************************************************************************************/

	IPKey::IPKey()
	{
		m_ip = 0;
		m_mask = 0xFFFFFFFF;
	}

	IPKey::IPKey(QString& ip, Uint32 mask)
			: m_mask(mask)
	{
		bool ok;
		this->m_ip = toUint32(ip, &ok);
	}

	IPKey::IPKey(Uint32 ip, Uint32 mask)
			: m_ip(ip), m_mask(mask)
	{}

	bool IPKey::operator ==(const IPKey& ip) const
	{
		return  (m_ip & m_mask) == m_mask & ip.m_ip;
	}

	bool IPKey::operator !=(const IPKey& ip) const
	{
		return (m_ip & m_mask) != m_mask & ip.m_ip;
	}

	bool IPKey::operator < (const IPKey& ip) const
	{
		return (m_ip & m_mask) < (m_mask & ip.m_ip);
	}

	IPKey& IPKey::operator =(const IPKey& ip)
	{
		m_ip = ip.m_ip;
		m_mask = ip.m_mask;
		return *this;
	}

	IPKey::~ IPKey()
{}}

