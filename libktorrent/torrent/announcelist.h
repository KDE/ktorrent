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
#ifndef BTANNOUNCELIST_H
#define BTANNOUNCELIST_H

#if 0
#include <kurl.h>
#include <qstring.h>
#include <interfaces/trackerslist.h>

namespace bt
{
	class BNode;

	/**
	 * @author Joris Guisson
	 * @brief Keep track of a list of trackers
	 * 
	 * This class keeps track of a list of tracker URL. 
	*/
	class AnnounceList : public kt::TrackersList
	{
		KURL::List trackers;
		KURL::List custom_trackers;
		
	public:
		AnnounceList();
		virtual ~AnnounceList();

		/**
		 * Load the list from a bencoded list of lists.
		 * @param node The BNode
		 */
		void load(BNode* node);
		
		/**
		 * Get a new tracker url.
		 * @param last_was_succesfull Wether or not the last url was succesfull
		 * @return An URL
		 */
		KURL getTrackerURL(bool last_was_succesfull) const;
		
		
		///Gets a list of trackers (URLs)
		const KURL::List getTrackerURLs();
		
		///Adds new tracker URL to the list
		void addTracker(KURL url, bool custom = true);
		
		/**
		 * Removes a tracker from the list
		 * @param url Tracker URL to remove from custom trackers list.
		 * @returns TRUE if URL is in custom list and it is removed or FALSE if it could not be removed or it's a default tracker
		 */
		bool removeTracker(KURL url);
		
		///Changes current tracker
		void setTracker(KURL url);
		
		///Restores the default torrent tracker
		void restoreDefault();

		/// Get the number of tracker URLs
		unsigned int getNumTrackerURLs() const {return trackers.count();}

		void debugPrintURLList();
		
		///Saves custom trackers in a file
		void saveTrackers();
		
		///Loads custom trackers from a file
		void loadTrackers();

		void setDatadir(const QString& theValue);
		
		/**
		 * Merge an other announce list to this one.
		 * @param al The AnnounceList
		 */
		void merge(const AnnounceList* al);
		
	private:
		QString m_datadir;
		
	};

}
#endif

#endif
