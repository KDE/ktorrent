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

#include "address.h"

using namespace KNetwork;

namespace net
{
	Address Address::null;

	Address::Address()
	{
	}
	
	Address::Address(const QString & host,Uint16 port) : KInetSocketAddress(KIpAddress(host),port)
	{
	}
	
	Address::Address(const Address & addr) : KInetSocketAddress(addr)
	{
	}
	
	Address::Address(const KNetwork::KInetSocketAddress & addr) : KInetSocketAddress(addr)
	{}
	
	Address:: ~Address()
	{}

/*	
	Address & Address::operator = (const Address & a)
	{
		m_ip = a.ip();
		m_port = a.port();
		return *this;
	}

	
	bool Address::operator == (const Address & a)
	{
		return m_ip == a.ip() && m_port == a.port();
	}

	QString Address::toString() const
	{
		return QString("%1.%2.%3.%4")
				.arg((m_ip & 0xFF000000) >> 24)
				.arg((m_ip & 0x00FF0000) >> 16)
				.arg((m_ip & 0x0000FF00) >> 8)
				.arg(m_ip & 0x000000FF);
	}
*/
}
