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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#ifndef BTTRACKER_H
#define BTTRACKER_H

#include <qtimer.h>
#include "globals.h"
#include "peerid.h"
#include <util/sha1hash.h>

class KURL;

namespace kt
{
	class TorrentInterface;
}

namespace dht
{
	class DHTTrackerBackend;
}

namespace bt
{
	class TorrentControl;
	class PeerManager;
	class Tracker;
	
	
	/**
	 * Backend class, does the actual communication with the tracker.
	*/
	class TrackerBackend : public QObject
	{
		Q_OBJECT
				
		
	public:
		TrackerBackend(Tracker* trk);
		virtual ~TrackerBackend();
		
		/**
		 * Do a request to the tracker.
		 * @param url The path and query
		 */
		virtual void doRequest(const KURL & url) = 0;

		/**
		 * Update all the data. If something is wrong in this function,
		 * an Error should be thrown.
		 * @param pman The PeerManager
		 */
		virtual void updateData(PeerManager* pman) = 0;
	protected:
		Tracker* frontend;
	};

	/**
	 * @author Joris Guisson
	 * @brief Communicates with the tracker
	 * 
	 * Class to communicate with the tracker. This class acts as a frontend for
	 * 2 possible backends (one for UDP one for HTTP), depending on the URL it will decide what backend
	 * to use.
	 *
	 * Once the data comes in, the Tracker should emit a signal.
	 */
	class Tracker : public QObject
	{
		Q_OBJECT
	public:
		/**
		 * Constructor.
		 */
		Tracker(kt::TorrentInterface* tor,const SHA1Hash & ih,const PeerID & pid);
		virtual ~Tracker();

		/// Get the number of seeders
		Uint32 getNumSeeders() const {return seeders;}

		/// Get the number of leechers
		Uint32 getNumLeechers() const {return leechers;}
		
		/**
		 * Do a request to the tracker.
		 * @param url The path and query
		 */
		void doRequest(const KURL & url);

		/**
		 * Update all the data. If something is wrong in this function,
		 * an Error should be thrown.
		 * @param pman The PeerManager
		 */
		void updateData(PeerManager* pman);
		
		/**
		 * Set the custom IP
		 * @param str 
		 */
		static void setCustomIP(const QString & str);
		
		/**
		 * Start the tracker.
		 */
		void start();
		
		/**
		 * Set the interval between attempts.
		 * @param secs Number of secs
		 */
		void setInterval(Uint32 secs);

		/**
		 * Get the interval between attempts in seconds.
		 * @param secs Number of secs
		 */
		Uint32 getInterval() const {return interval;}
		
		/**
		 * Stop the tracker.
		 */
		void stop();

		/**
		 * Manually update the tracker.
		 */
		void manualUpdate();

		/**
		 * Alert the tracker that an error occured. 
		 */
		void handleError();

		/**
		 * The download was completed, alert the tracker of this
		 * fact.
		 */
		void completed();

		/// Get the time to the next update in seconds.
		Uint32 getTimeToNextUpdate() const;
	signals:
		/**
		 * Signal to notify of errors.
		 */
		void error();
		
		/**
		 * Update succeeded data has been recieved.
		 */
		void dataReady();

	private slots:
		void onTimeout();
		void onErrorTimeout();
		
	protected:
		void updateOK();
		void emitError() {error();}
		void emitDataReady() {dataReady();}
		

	protected:
		SHA1Hash info_hash;
		PeerID peer_id;
		Uint32 seeders,leechers;
		QString event;
		Uint32 interval;
		kt::TorrentInterface* tor;
		QTimer update_timer,error_update_timer;
		Uint32 time_of_last_update;
		Uint32 num_failed_attempts;
		bool error_mode;
		Uint32 key;
		static QString custom_ip,custom_ip_resolved;
		TrackerBackend* udp;
		TrackerBackend* http;
		TrackerBackend* curr;
		TrackerBackend* dht_ba;
		
		friend class UDPTracker;
		friend class HTTPTracker;
		friend class dht::DHTTrackerBackend;
	};

}

#endif
