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
#ifndef BTANNOUNCELIST_H
#define BTANNOUNCELIST_H

#include <kurl.h>

namespace bt
{
	class BNode;

	/**
	 * @author Joris Guisson
	 * @brief Keep track of a list of trackers
	 * 
	 * This class keeps track of a list of tracker URL. Whenever the update
	 * of Tracker failed, a new URL will be asked from this class.
	*/
	class AnnounceList
	{
		KURL::List trackers;
		mutable int curr;
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

		/// Get the number of tracker URLs
		unsigned int getNumTrackerURLs() const {return trackers.count();}
	};

}

#endif
