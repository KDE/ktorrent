/***************************************************************************
 *   Copyright (C) 2009 by Joris Guisson                                   *
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

#ifndef BT_TRACKERMANAGER_H
#define BT_TRACKERMANAGER_H

#include <QTimer>
#include <QDateTime>
#include <btcore_export.h>
#include <util/ptrmap.h>
#include <util/constants.h>
#include <tracker/tracker.h>
#include <interfaces/trackerslist.h>


namespace bt 
{
	class TorrentControl;
	class WaitJob;
	class PeerManager;
	
	/**
	 * Manages all trackers
	 */
	class BTCORE_EXPORT TrackerManager : public QObject,public bt::TrackersList,public TrackerDataSource
	{
		Q_OBJECT
	public:
		TrackerManager(TorrentControl* tor,PeerManager* pman);
		virtual ~TrackerManager();
		
		virtual TrackerInterface* getCurrentTracker() const;
		virtual void setCurrentTracker(TrackerInterface* t);
		virtual void setCurrentTracker(const KUrl & url);
		virtual QList<TrackerInterface*> getTrackers();
		virtual TrackerInterface* addTracker(const KUrl &url, bool custom = true,int tier = 1);
		virtual bool removeTracker(TrackerInterface* t);
		virtual bool removeTracker(const KUrl & url);
		virtual bool canRemoveTracker(TrackerInterface* t);
		virtual void restoreDefault();
		virtual void setTrackerEnabled(const KUrl & url,bool on);
		virtual bool noTrackersReachable() const;
		
		/// Get the number of seeders
		Uint32 getNumSeeders() const;
		
		/// Get the number of leechers
		Uint32 getNumLeechers() const;
		
		/**
		* Start gathering peers
		*/
		virtual void start();
		
		/**
		* Stop gathering peers
		* @param wjob WaitJob to wait at exit for the completion of stopped events to the trackers
		*/
		virtual void stop(WaitJob* wjob = 0);
		
		/**
		* Notify peersources and trackrs that the download is complete.
		*/
		virtual void completed();
		
		/**
		* Do a manual update on all peer sources and trackers.
		*/
		virtual void manualUpdate();
		
		/**
		* Do a scrape on the current tracker
		* */
		virtual void scrape();
		
	protected:
		void saveCustomURLs();
		void loadCustomURLs();
		void saveTrackerStatus();
		void loadTrackerStatus();
		void addTracker(Tracker* trk);
		void switchTracker(Tracker* trk);
		Tracker* selectTracker();
		
		virtual Uint64 bytesDownloaded() const;
		virtual Uint64 bytesUploaded() const;
		virtual Uint64 bytesLeft() const;
		virtual const SHA1Hash & infoHash() const;
		
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
		* Update the current tracker manually
		*/
		void updateCurrentManually();
		
	protected:
		TorrentControl* tor;
		PtrMap<KUrl,Tracker> trackers;
		bool no_save_custom_trackers;
		PeerManager* pman;
		Tracker* curr;
		KUrl::List custom_trackers;
		bool started;
	};

}

#endif // BT_TRACKERMANAGER_H
