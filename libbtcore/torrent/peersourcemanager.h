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
#ifndef BTPEERSOURCEMANAGER_H
#define BTPEERSOURCEMANAGER_H


#include <util/constants.h>
#include <util/waitjob.h>
#include <tracker/tracker.h>
#include <tracker/trackermanager.h>
#include <interfaces/torrentinterface.h>



namespace dht
{
	class DHTPeerSource;
}

namespace bt
{
	class Torrent;
	class TorrentControl;
	class PeerSource;

	/**
	 * @author Joris Guisson <joris.guisson@gmail.com>
	 * 
	 * This class manages all PeerSources.
	*/
	class PeerSourceManager : public TrackerManager
	{
		Q_OBJECT
		
		QList<PeerSource*> additional;
		dht::DHTPeerSource* m_dht;
		
	public:
		PeerSourceManager(TorrentControl* tor,PeerManager* pman);
		virtual ~PeerSourceManager();
	
				
		/**
		 * Add a PeerSource, the difference between PeerSource and Tracker
		 * is that only one Tracker can be used at the same time, 
		 * PeerSource can always be used.
		 * @param ps The PeerSource
		 */
		void addPeerSource(PeerSource* ps);
		
		/**
		* Remove a Tracker or PeerSource.
		* @param ps 
		*/
		void removePeerSource(PeerSource* ps);

		/**
		 * See if the PeerSourceManager has been started 
		 */
		bool isStarted() const {return started;}
		
	
		virtual void start();
		virtual void stop(WaitJob* wjob = 0);
		virtual void completed();
		virtual void manualUpdate();
		
		///Adds DHT as PeerSource for this torrent
		void addDHT();
		///Removes DHT from PeerSourceManager for this torrent.
		void removeDHT();
		///Checks if DHT is enabled
		bool dhtStarted();
	};

}

#endif
