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

namespace bt
{
	IPBlocklist::IPBlocklist()
	{
		insert("0.0.0.0",3);
	}
	
	IPBlocklist::IPBlocklist(const IPBlocklist & ) {}

	void IPBlocklist::insert( QString ip, int state )
	{
		if (m_peers.contains(ip))
			m_peers[ip]+= state;
		else
			m_peers.insert(ip,state);
	}
	
	bool IPBlocklist::isBlocked( QString& ip )
	{
		if (!m_peers.contains(ip))
			return false;
		
		return m_peers[ip] >= 3;
	}
}

