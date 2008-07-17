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
 
#ifndef IPBLOCKINGINTERFACE_H
#define IPBLOCKINGINTERFACE_H

#include <btcore_export.h>

namespace net
{
	class Address;
}


namespace bt
{
	/**
	 * @author Ivan Vasic
	 * @brief Base class for BlockLists
	*/
	class BTCORE_EXPORT BlockListInterface 
	{	
	public:
		BlockListInterface();
		virtual ~BlockListInterface();
		
		/**
		 * This function checks if IP is blocked
		 * @return TRUE if IP should be blocked. FALSE otherwise
		 * @arg addr Address of the peer
		 */
		virtual bool isBlockedIP(const net::Address & addr) = 0;
			
		
		/**
		 * Same as above, accept this takes a string as parameter
		 * @param addr The IP address represented as a string
		 * @return TRUE if IP should be blocked. FALSE otherwise
		 */
		virtual bool isBlockedIP(const QString & addr) = 0;
	};
}
#endif
