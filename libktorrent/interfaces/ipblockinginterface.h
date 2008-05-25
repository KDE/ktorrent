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

class QString;

namespace kt
{
	/**
	 * @author Ivan Vasic
	 * @brief Interface for IPBlocklist to communicate with IPBlockingPlugin
	*/
	class IPBlockingInterface
	{	
		public:
    		IPBlockingInterface();
    		virtual ~IPBlockingInterface();
			
			/**
			 * This function checks if IP is listed in antip2p filter list.
			 * @return TRUE if IP should be blocked. FALSE otherwise
			 * @arg ip String representation of IP address.
			 */
			virtual bool isBlockedIP(const QString& ip) = 0;
			
	};
}
#endif
