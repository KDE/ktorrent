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

#include <qtimer.h>
#include <qdatetime.h>
#include <qlist.h>
#include <util/ptrmap.h>
#include <util/constants.h>
#include <util/waitjob.h>
#include <tracker/tracker.h>
#include <interfaces/trackerslist.h>
#include <interfaces/torrentinterface.h>



namespace dht
{
	class DHTTrackerBackend;
}

namespace bt
{
	class PeerManager;
	class Torrent;
	class TorrentControl;
	class PeerSource;

	/**
	 * @author Joris Guisson <joris.guisson@gmail.com>
	 * 
	 * This class manages all PeerSources.
	*/
	class PeerSourceManager : public QObject, public TrackersList
	{
		Q_OBJECT
				
		TorrentControl* tor;
		PeerManager* pman;
		PtrMap<KUrl,Tracker> trackers;
		QList<PeerSource*> additional;
		Tracker* curr;
		dht::DHTTrackerBackend* m_dht;
		bool started;
		bool pending;
		KUrl::List custom_trackers;
		QDateTime request_time;
		QTimer timer;
		Uint32 failures;
		bool no_save_custom_trackers;
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
		 * See if the PeerSourceManager has been started 
		 */
		bool isStarted() const {return started;}
		
		/**
		 * Start gathering peers
		 */
		void start();
		
		/**
		 * Stop gathering peers
		 * @param wjob WaitJob to wait at exit for the completion of stopped events to the trackers
		 */
		void stop(WaitJob* wjob = 0);
		
		/**
		 * Notify peersources and trackrs that the download is complete.
		 */
		void completed();
		
		/**
		 * Do a manual update on all peer sources and trackers.
		 */
		void manualUpdate();

		/**
		 * Do a scrape on the current tracker
		 * */
		void scrape();
		
		/**
		 * Remove a Tracker or PeerSource.
		 * @param ps 
		 */
		void removePeerSource(PeerSource* ps);
		
		virtual KUrl getTrackerURL() const;
		virtual KUrl::List getTrackerURLs();
		virtual void addTracker(const KUrl &url, bool custom = true,int tier = 1);
		virtual bool removeTracker(const KUrl &url);
		virtual void setTracker(const KUrl &url);
		virtual void restoreDefault();
		virtual void setTrackerEnabled(const KUrl & url,bool enabled);
		virtual bool isTrackerEnabled(const KUrl & url) const;
		
		/**
		 * Get the time to the next tracker update.
		 * @return The time in seconds
		 */
		Uint32 getTimeToNextUpdate() const;
		
		/// Get the number of potential seeders
		Uint32 getNumSeeders() const;
		
		/// Get the number of potential leechers
		Uint32 getNumLeechers() const;
		
		/// Get the total time the torrent was downloaded according to the tracker
		Uint32 getTotalTimesDownloaded() const;
		
		/// Get the number of failures
		Uint32 getNumFailures() const {return failures;}
		
		///Adds DHT as PeerSource for this torrent
		void addDHT();
		///Removes DHT from PeerSourceManager for this torrent.
		void removeDHT();
		///Checks if DHT is enabled
		bool dhtStarted();
		
	private slots:
		/**
		 * The an error happened contacting the tracker.
		 * @param err The error
		 */
		void onTrackerError(const QString & err);
		
		/**
		 * Tracker update was OK.
		 * @param 
		 */
		void onTrackerOK();
		
		/**
		 * Tracker is doing a request.  
		 */
		void onTrackerRequestPending();
		
		/**
		 * Update the current tracker manually
		 */
		void updateCurrentManually();
		
	signals:
		/**
		 * Status has changed of the tracker.
		 * @param status The tracker status
		 * @param msg Message to show to user
		 */
		void statusChanged(TrackerStatus status,const QString & msg);
		
	private:
		void saveCustomURLs();
		void loadCustomURLs();
		void saveTrackerStatus();
		void loadTrackerStatus();
		void addTracker(Tracker* trk);
		void switchTracker(Tracker* trk);
		Tracker* selectTracker();
	};

}

#endif
