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
#ifndef NETADDRESS_H
#define NETADDRESS_H

#include <qstring.h>
#include <util/constants.h>

namespace net
{
	using bt::Uint32;
	using bt::Uint16;

	/**
		@author Joris Guisson <joris.guisson@gmail.com>
	*/
	class Address
	{
		Uint32 m_ip;
		Uint16 m_port;
	public:
		Address();
		Address(const QString & host,Uint16 port);
		Address(const Address & addr);
		virtual ~Address();

	
		Address & operator = (const Address & a);
		bool operator == (const Address & a);
		
		Uint32 ip() const {return m_ip;}
		void setIP(Uint32 ip) {m_ip = ip;}
		
		Uint16 port() const {return m_port;}
		void setPort(Uint16 p) {m_port = p;}

		QString toString() const;

	};

}

#endif
