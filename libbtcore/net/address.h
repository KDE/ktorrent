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
#include <k3socketaddress.h>
#include <btcore_export.h>

namespace net
{
	using bt::Uint32;
	using bt::Uint16;

	/**
	 * @author Joris Guisson <joris.guisson@gmail.com>
	 * 
	 * Network address, contains an IP address and a port number. 
	 * This supports both IPv4 and IPv6 addresses.
	*/
	class BTCORE_EXPORT Address : public KNetwork::KInetSocketAddress
	{
	public:
		Address();	
		Address(const QString & host,Uint16 port);
		Address(const KNetwork::KInetSocketAddress & addr);
		Address(const Address & addr);
		virtual ~Address();

		
		static Address null;
	};

}

#endif
