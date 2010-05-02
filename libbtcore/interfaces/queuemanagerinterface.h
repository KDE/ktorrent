/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#ifndef BTQUEUEMANAGERINTERFACE_H
#define BTQUEUEMANAGERINTERFACE_H

#include <btcore_export.h>

namespace bt
{
	class SHA1Hash;
	struct TrackerTier;

	/**
		@author
	*/
	class BTCORE_EXPORT QueueManagerInterface
	{
		static bool qm_enabled;
	public:
		QueueManagerInterface();
		virtual ~QueueManagerInterface();

		/**
			* See if we already loaded a torrent.
			* @param ih The info hash of a torrent
			* @return true if we do, false if we don't
			*/
		virtual bool alreadyLoaded(const SHA1Hash & ih) const = 0;


		/**
			* Merge announce lists to a torrent
			* @param ih The info_hash of the torrent to merge to
			* @param trk First tier of trackers
			*/
		virtual void mergeAnnounceList(const SHA1Hash & ih,const TrackerTier* trk) = 0;
		
		/**
		 * Disable or enable the QM
		 * @param on 
		 */
		static void setQueueManagerEnabled(bool on);
		
		static bool enabled() {return qm_enabled;}
	};

}

#endif
