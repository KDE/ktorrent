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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.           *
 ***************************************************************************/

#include "ipblocklist.h"
#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>
#include <util/constants.h>
#include <util/log.h>
#include "globals.h"
#include <interfaces/ipblockinginterface.h>


namespace bt
{
	Uint32 toUint32(const QString& ip, bool* ok)
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
		this->pluginInterface = 0;
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
		Out(SYS_IPF|LOG_NOTICE) << "IP " << ip << " banned." << endl;
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
	
	void IPBlocklist::removeRange(QString ip)
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
		
		QMap<IPKey, int>::iterator it = m_peers.find(key);
		if (it == m_peers.end())
			return;
		
		m_peers.remove(key);
	}
	
	void IPBlocklist::setPluginInterfacePtr( kt::IPBlockingInterface* ptr )
	{
		this->pluginInterface = ptr;
	}

	bool IPBlocklist::isBlocked(const QString& ip )
	{
		//First check local filter list
		if(isBlockedLocal(ip))
		{
			Out(SYS_IPF|LOG_NOTICE) << "IP " << ip << " is blacklisted. Connection denied." << endl;
			return true;
		}
		
		//Then we ask plugin
		if(isBlockedPlugin(ip))
		{
			Out(SYS_IPF|LOG_NOTICE) << "IP " << ip << " is blacklisted. Connection denied." << endl;
			return true;
		}

		return false;
	}
	
	bool IPBlocklist::isBlockedLocal(const QString& ip )
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
	
	bool IPBlocklist::isBlockedPlugin(const QString& ip )
	{
		if (pluginInterface == 0) //the plugin is not loaded
			return false;
		else
			return pluginInterface->isBlockedIP(ip);
	}
	
	QStringList* IPBlocklist::getBlocklist()
	{
		QStringList* ret = new QStringList();
		QMap<IPKey,int>::iterator it = m_peers.begin();
		for( ;it!=m_peers.end();++it)
		{
			IPKey key = it.key();
			*ret << key.toString();
		}
		
		return ret;
	}
	
	void IPBlocklist::setBlocklist(QStringList* list)
	{
		m_peers.clear();
		for (QStringList::Iterator it = list->begin(); it != list->end(); ++it ) 
			addRange(*it);
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
	
	IPKey::IPKey(const IPKey& ip)
	{
		m_ip = ip.m_ip;
		m_mask = ip.m_mask;
	}

	IPKey::IPKey(Uint32 ip, Uint32 mask)
			: m_ip(ip), m_mask(mask)
	{}

	QString IPKey::toString()
	{
		Uint32 tmp, tmpmask;
		Uint32 ip = m_ip;
		Uint32 mask = m_mask;
		QString out;
	
		tmp = ip;
		tmpmask = mask;
		tmp &= 0x000000FF;
		tmpmask &= 0x000000FF;
		if(tmpmask == 0)
			out.prepend("*");
		else
			out.prepend(QString("%1").arg(tmp));
		ip >>= 8;
		mask >>= 8;
		tmp = ip;
		tmpmask = mask;
		tmp &= 0x000000FF;
		tmpmask &= 0x000000FF;
		if(tmpmask == 0)
			out.prepend("*.");
		else
			out.prepend(QString("%1.").arg(tmp));
		ip >>= 8;
		mask >>= 8;
		tmp = ip;
		tmpmask = mask;
		tmp &= 0x000000FF;
		tmpmask &= 0x000000FF;
		if(tmpmask == 0)
			out.prepend("*.");
		else
			out.prepend(QString("%1.").arg(tmp));
		ip >>= 8;
		mask >>= 8;
		tmp = ip;
		tmpmask = mask;
		tmp &= 0x000000FF;
		tmpmask &= 0x000000FF;
		if(tmpmask == 0)
			out.prepend("*.");
		else
			out.prepend(QString("%1.").arg(tmp));
	
		return out;
	}
	
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
	{}
}
