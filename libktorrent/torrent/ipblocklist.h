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
#ifndef IPBLOCKLIST_H
#define IPBLOCKLIST_H

#include <interfaces/ipblockinginterface.h>

#include <qmap.h>
#include <qstringlist.h>
#include <util/constants.h>

class QString;

namespace bt
{
	class IPKey
	{
		public:
			IPKey();
			IPKey(QString& ip, Uint32 mask = 0xFFFFFFFF);
			IPKey(Uint32 ip, Uint32 mask = 0xFFFFFFFF);
			IPKey(const IPKey& ip);
			~IPKey();

			bool operator== (const IPKey& ip) const;
			bool operator!= (const IPKey& ip) const;
			bool operator < (const IPKey & ip) const;
			IPKey&  operator= (const IPKey& ip);
			
			QString toString();

			Uint32 m_ip;
			Uint32 m_mask;
	};

	/**
	 * @author Ivan Vasic <ivasic@gmail.com>
	 * @brief Keeps track of blocked peers
	 *
	 * This class is used for keeping the IP addresses list of peers that
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
			* @brief Adds IP range to the list
			* It is used  for blocking plugin. For single IP use insert() instead.
			* @param ip QString peer IP address. Uses ''*" for ranges.
			 **/
			void addRange(QString ip);
			
			
			/**
			 * @brief Removes IP range from list
			 * It is used  for blocking plugin.
			 * @param ip QString peer IP address. Uses ''*" for ranges.
			 **/
			void removeRange(QString ip);

			/**
			 * Checks if IP is in the blocking list
			 * @param ip - IP address to check
			 * @returns true if IP is blocked
			 */
			bool isBlocked(const QString& ip);
			
			/**
			 * @brief Sets the pointer to the IPBlockingInterface (IPBlocking plugin)
			 * Call this function from IPBlocking plugin when it gets loaded.
			 * @arg ptr - pointer to be set
			 */
			void setPluginInterfacePtr(kt::IPBlockingInterface* ptr);
			
			/**
			 * @brief Unsets the interface pointer
			 * Call this when IPBlockingPlugin gets unloaded or deleted
			 */
			void unsetPluginInterfacePtr() { pluginInterface = 0; }
			
			
			/**
			 * @brief This function will fill QStringList with all banned peer IP addresses.
			 * @return QStringList filled with blacklisted peers. 
			 * It will create a new QStringList object so don't forget to delete it after using.
			 */
			QStringList* getBlocklist();
			
			
			/**
			 * @brief This function will load blacklisted peers to IPFilter.
			 * @param list QStringList containing all banned peers.
			 * @note This function will remove current peers from blocklist before setting new list!!!
			 */
			void setBlocklist(QStringList* list);

		private:
			
			/**
			 * Pointer to the IPBlocking plugin which implements IPBlockingInterface
			 * Used to provide a way to use this plugin functions from within this class
			 */
			kt::IPBlockingInterface* pluginInterface;
			
			/**
			 * @param IPKey - Key: Peer IP address and bit mask if it is a range
			 * @param int - Number of bad chunks sent.
			**/
			QMap<IPKey, int> m_peers;
			
			/**
			 * @brief Adds IP range to the list.
			 * @param key IPKey that represents this IP range
			 * @param state int Number of 'warnings' for the range. 
			 * Default is 3 - that means range is blocked permanently.
			*/
			void insertRangeIP(IPKey& key, int state=3);
			
			
			/**
			 * Checks if IP is listed in local database (IPBlocklist::m_peers)
			 * @return TRUE if IP is to be blocked
			 */
			bool isBlockedLocal(const QString& ip);
			
			/**
			 * Checks if IP is listed in plugins antip2p file
			 * @return TRUE if IP is to be blocked
			 */
			bool isBlockedPlugin(const QString& ip);
	};
}

#endif

