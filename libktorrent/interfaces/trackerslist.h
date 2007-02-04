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
#ifndef KTTRACKERSLIST_H
#define KTTRACKERSLIST_H

#include <kurl.h>

namespace bt
{
	struct TrackerTier;
}

namespace kt
{
	/**
	 * @author Ivan Vasić <ivasic@gmail.com>
	 * 
	 * This interface is used to provide access to AnnounceList object which holds a list of available trackers for a torrent.
	*/
	class TrackersList
	{
		public:
			TrackersList();
			virtual ~TrackersList();
			
			/**
			 * Get the current tracker URL.
			 */
			virtual KURL getTrackerURL() const = 0;
			
			/**
			 * Gets a list of available trackers.
			 */
			virtual KURL::List getTrackerURLs() = 0;
		
			/**
			 * Adds a tracker URL to the list.
			 * @param url The URL
			 * @param custom Is it a custom tracker
			 * @param tier Which tier (or priority) the tracker has, tier 1 are 
			 * the main trackers, tier 2 are backups ...
			 */
			virtual void addTracker(KURL url, bool custom = true,int tier = 1) = 0;
		
			/**
			 * Removes the tracker from the list.
			 * @param url - Tracker url.
			 */
			virtual bool removeTracker(KURL url) = 0;
		
			/**
			 * Sets the current tracker and does the announce.
			 * @param url - Tracker url.
			 */
			virtual void setTracker(KURL url) = 0;
			
			/**
			 * Restores the default tracker and does the announce.
			 */
			virtual void restoreDefault() = 0;
			
			/**
			 * Merge an other tracker list.
			 * @param first The first TrackerTier
			 */
			void merge(const bt::TrackerTier* first);

	};

}

#endif
