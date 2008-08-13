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
#ifndef BTTRACKER_H
#define BTTRACKER_H

#include <kurl.h>
#include <util/sha1hash.h>
#include <interfaces/peersource.h>
#include <peer/peerid.h>
#include <btcore_export.h>

class KUrl;

namespace bt
{
	class TorrentInterface;
	
	/**
	 * Base class for all tracker classes.
	*/
	class BTCORE_EXPORT Tracker : public PeerSource
	{
		Q_OBJECT
	public:
		Tracker(const KUrl & url,TorrentInterface* tor,const PeerID & id,int tier);
		virtual ~Tracker();
		
		/// See if a start request succeeded
		bool isStarted() const {return started;}
		
		/**
		 * Set the custom IP
		 * @param str 
		 */
		static void setCustomIP(const QString & str);
		
		/// get the tracker url
		KUrl trackerURL() const {return url;}
		
		/**
		 * Delete the tracker in ms milliseconds, or when the stopDone signal is emitted.
		 * @param ms Number of ms to wait
		 */
		void timedDelete(int ms);
		
		/**
		 * Get the number of failed attempts to reach a tracker.
		 * @return The number of failed attempts
		 */
		virtual Uint32 failureCount() const = 0;
		
		/**
		 * Do a tracker scrape to get more accurate stats about a torrent.
		 * Does nothing if the tracker does not support this.
		 */
		virtual void scrape() = 0;
		
		/// Get the trackers tier
		int getTier() const {return tier;}
		
		/**
		 * Get the update interval in ms
		 * @return interval
		 */
		Uint32 getInterval() const {return interval;}
		
		/// Set the interval
		void setInterval(Uint32 i) {interval = i;}
		
		/// Get the number of seeders
		Uint32 getNumSeeders() const {return seeders;}
		
		/// Get the number of leechers
		Uint32 getNumLeechers() const {return leechers;}
		
		/// Get the number of times the torrent was downloaded
		Uint32 getTotalTimesDownloaded() const {return total_downloaded;}
		
		/// Get the custom ip to use, null if none is set
		static QString getCustomIP();
		
		/// Enable or disable the tracker
		void setEnabled(bool on) {enabled = on;}
		
		/// Is the tracker enabled
		bool isEnabled() const {return enabled;}
	signals:
		/**
		 * Emitted when an error happens.
		 * @param failure_reason The reason why we couldn't reach the tracker
		 */
		void requestFailed(const QString & failure_reason);
		
		/**
		 * Emitted when a stop is done.
		 */
		void stopDone();
		
		/**
		 * Emitted when a request to the tracker succeeded
		 */
		void requestOK();
		
		/**
		 * A request to the tracker has been started.
		 */
		void requestPending();

		/**
		 * Emitted when a scrape has finished
		 * */
		void scrapeDone();
		
	protected:
		KUrl url;
		int tier;
		PeerID peer_id;
		TorrentInterface* tor;
		Uint32 interval,seeders,leechers,key,total_downloaded;
		bool started;
		bool enabled;
	};
	
}

#endif
