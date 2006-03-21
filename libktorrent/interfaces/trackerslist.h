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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#ifndef KTTRACKERSLIST_H
#define KTTRACKERSLIST_H

#include <kurl.h>

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
			 * Gets the tracker URL.
			 * @param last_was_succesfull Weather last connect attempt was successfull (if not the next available tracker URL is returned).
			 */
			virtual KURL getTrackerURL(bool last_was_succesfull) const = 0;
			
			/**
			 * Gets a list of available trackers.
			 */
			virtual const KURL::List getTrackerURLs() = 0;
		
			/**
			 * Adds a tracker URL to the list.
			 * @param custom - Weather or not the user added this tracker. Most of the time this is the case.
			 */
			virtual void addTracker(KURL url, bool custom = true) = 0;
		
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

	};

}

#endif
