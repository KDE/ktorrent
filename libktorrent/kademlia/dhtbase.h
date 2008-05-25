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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#ifndef DHTDHTBASE_H
#define DHTDHTBASE_H

#include <qobject.h>
#include <util/constants.h>

class QString;

namespace bt
{
	class SHA1Hash;
}

namespace dht
{
	class AnnounceTask;
	
	struct Stats
	{
		/// number of peers in the routing table
		bt::Uint32 num_peers;
		/// Number of running tasks
		bt::Uint32 num_tasks;
	};

	/**
	 * @author Joris Guisson <joris.guisson@gmail.com>
	 * 
	 * Interface for DHT class, this is to keep other things separate from the inner workings
	 * of the DHT.
	 */
	class DHTBase : public QObject
	{
		Q_OBJECT
	public:
		DHTBase();
		virtual ~DHTBase();
		
		
		/**
		 * Start the DHT
		 * @param table File where the save table is located
		 * @param key_file The file where the key is stored
		 * @param port The port to use
		 */
		virtual void start(const QString & table,const QString & key_file,bt::Uint16 port) = 0;
		
		/**
		 * Stop the DHT
		 */
		virtual void stop() = 0;
		
		/**
		 * Update the DHT
		 */
		virtual void update() = 0;
		
		/**
		 * A Peer has received a PORT message, and uses this function to alert the DHT of it.
		 * @param ip The IP of the peer
		 * @param port The port in the PORT message
		 */
		virtual void portRecieved(const QString & ip,bt::Uint16 port) = 0;
		
		/**
		 * Do an announce on the DHT network
		 * @param info_hash The info_hash
		 * @param port The port
		 * @return The task which handles this
		 */
		virtual AnnounceTask* announce(const bt::SHA1Hash & info_hash,bt::Uint16 port) = 0;
		
		/**
		 * See if the DHT is running.
		 */
		bool isRunning() const {return running;}
		
		/// Get the DHT port
		bt::Uint16 getPort() const {return port;}
		
		/// Get statistics about the DHT
		const dht::Stats & getStats() const {return stats;}
		
		/**
		 * Add a DHT node. This node shall be pinged immediately.
		 * @param host The hostname or ip
		 * @param hport The port of the host
		 */
		virtual void addDHTNode(const QString & host,bt::Uint16 hport) = 0;
		
		/**
		 * Returns maxNodes number of <IP address, port> nodes 
		 * that are closest to ourselves and are good.
		 * @param maxNodes maximum nr of nodes in QMap to return.
		 */
		virtual QMap<QString, int> getClosestGoodNodes(int maxNodes) = 0;
		
	signals:
		void started();
		void stopped();
		
	protected:
		bool running;
		bt::Uint16 port;
		dht::Stats stats;
	};

}

#endif
