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
#ifndef KTPEERSOURCE_H
#define KTPEERSOURCE_H

#include <qobject.h>
#include <qvaluelist.h>
#include <util/constants.h>
		
namespace bt
{
	class WaitJob;
}

namespace kt
{
	struct PotentialPeer
	{
		QString ip;
		bt::Uint16 port;
		bool local;
		
		PotentialPeer() : port(0),local(false) {}
	};

	/**
	 * @author Joris Guisson <joris.guisson@gmail.com>
	 * 
	 * This class is the base class for all classes who which to provide potential peers
	 * for torrents. PeerSources should work independently and should emit a signal when they
	 * have peers ready.
	*/
	class PeerSource : public QObject
	{
		Q_OBJECT
	public:
		PeerSource();
		virtual ~PeerSource();


		
		/**
		 * Take the first PotentialPeer from the list. The item
		 * is removed from the list.
		 * @param pp PotentialPeer struct to fill
		 * @return true If there was one available, false if not
		 */
		bool takePotentialPeer(PotentialPeer & pp);
		
		/**
		 * Add a peer to the list of peers.
		 * @param ip The ip
		 * @param port The port
		 * @param local Wether or not the peer is on the local network
		 */
		void addPeer(const QString & ip,bt::Uint16 port,bool local = false);
		
	public slots:
		/**
		 * Start gathering peers.
		 */
		virtual void start() = 0;
		
		/**
		 * Stop gathering peers.
		 */
		virtual void stop(bt::WaitJob* wjob = 0) = 0;
		
		/**
		 * The torrent has finished downloading.
		 * This is optional and should be used by HTTP and UDP tracker sources
		 * to notify the tracker.
		 */
		virtual void completed();
		
		/**
		 * PeerSources wanting to implement a manual update, should implement this. 
		 */
		virtual void manualUpdate();
		
		/**
		 * The source is about to be destroyed. Subclasses can override this
		 * to clean up some things.
		 */
		virtual void aboutToBeDestroyed();
	signals:
		/**
		 * This signal should be emitted when a new batch of peers is ready.
		 * @param ps The PeerSource
		 */
		void peersReady(kt::PeerSource* ps);
		
	
	private:
		/// List to keep the potential peers in.
		QValueList<PotentialPeer> peers;	
	};

}

#endif
