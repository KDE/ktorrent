/***************************************************************************
 *   Copyright (C) 2005 by Ivan Vasic                                      *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#ifndef BTTRACKERSLIST_H
#define BTTRACKERSLIST_H

#include <kurl.h>
#include <btcore_export.h>

namespace bt
{
	struct TrackerTier;
	class TrackerInterface;

	/**
	 * @author Ivan Vasić <ivasic@gmail.com>
	 * 
	 * This interface is used to provide access to AnnounceList object which holds a list of available trackers for a torrent.
	*/
	class BTCORE_EXPORT TrackersList
	{
	public:
		TrackersList();
		virtual ~TrackersList();
		
		/**
		 * Get the current tracker (for non private torrents this returns 0, seeing that
		 * all trackers are used at the same time)
		 */
		virtual TrackerInterface* getCurrentTracker() const = 0;
		
		/**
		* Sets the current tracker and does the announce. For non private torrents, this
		* does nothing.
		* @param t The Tracker
		*/
		virtual void setCurrentTracker(TrackerInterface* t) = 0;
		
		/**
		* Sets the current tracker and does the announce. For non private torrents, this
		* does nothing.
		* @param url Url of the tracker
		*/
		virtual void setCurrentTracker(const KUrl & url) = 0;
		
		/**
		 * Gets a list of all available trackers. 
		 */
		virtual QList<TrackerInterface*> getTrackers() = 0;
	
		/**
		 * Adds a tracker URL to the list.
		 * @param url The URL
		 * @param custom Is it a custom tracker
		 * @param tier Which tier (or priority) the tracker has, tier 1 are 
		 * the main trackers, tier 2 are backups ...
		 * @return The Tracker
		 */
		virtual TrackerInterface* addTracker(const KUrl &url, bool custom = true,int tier = 1) = 0;
	
		/**
		 * Removes the tracker from the list.
		 * @param t The Tracker
		 */
		virtual bool removeTracker(TrackerInterface* t) = 0;
		
		/**
		 * Removes the tracker from the list.
		 * @param url The tracker url
		 */
		virtual bool removeTracker(const KUrl & url) = 0;
		
		/**
		 * Return true if a tracker can be removed
		 * @param t The tracker
		 */
		virtual bool canRemoveTracker(TrackerInterface* t) = 0;
		
		/**
		 * Restores the default tracker and does the announce.
		 */
		virtual void restoreDefault() = 0;
		
		/**
		 * Set a tracker enabled or not
		 */
		virtual void setTrackerEnabled(const KUrl & url,bool on) = 0;
		
		/**
		 * Merge an other tracker list.
		 * @param first The first TrackerTier
		 */
		void merge(const bt::TrackerTier* first);
		
		/**
		 * Returns true if no tracker is reachable
		 */
		virtual bool noTrackersReachable() const = 0;

	};

}

#endif
