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
#ifndef IPBLOCKLIST_H
#define IPBLOCKLIST_H



#include <qmap.h>

class QString;

namespace bt
{
	/**
	 * @author Ivan Vasic <ivasic@gmail.com>
	 * @brief Keeps track of blocked peers
	 *
	 * This class is used for keeping the IP adresses list of peers that
	 * have sent bad chunks.
	 *
	 * Peers that have sent >= 3 bad chunks are blocked.
	 */
	class IPBlocklist
	{
		IPBlocklist();
		IPBlocklist(const IPBlocklist & );
		const IPBlocklist& operator=(const IPBlocklist&);

	public:
		
		inline static IPBlocklist & instance() 
		{
			static IPBlocklist singleton;
			return singleton; 
		}
		
		/**
		 * @brief Adds ip address to the list. 
		 * It also increases the number of times this IP appeared in the list.
		 * @param ip QString containing the peer IP address
		 * @param state int number of bad chunks client from ip sent. Basically this parameter
		 * is used only to permanently block some IP (by setting this param to 3)
		 */
		void insert(QString ip, int state=1);
		
		/**
		 * Checks if IP is in the blocking list
		 * @param ip - IP address to check
		 * @returns true if IP is blocked
		 */
		bool isBlocked(QString& ip);

	private:
		/**
		 * @param QString - Key: Peer IP address.
		 * @param int - Number of bad chunks sent.
		 */
		QMap<QString, int> m_peers;
	};
}

#endif

