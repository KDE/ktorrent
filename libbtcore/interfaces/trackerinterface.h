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
#ifndef BT_TRACKERINTERFACE_H
#define BT_TRACKERINTERFACE_H

#include <QDateTime>
#include <kurl.h>
#include <btcore_export.h>
#include <util/constants.h>

namespace bt 
{
	enum TrackerStatus
	{
		TRACKER_OK,TRACKER_ANNOUNCING,TRACKER_ERROR,TRACKER_IDLE
	};
	
	/**
		Interface class for trackers to be used in plugins
	 */
	class BTCORE_EXPORT TrackerInterface
	{
	public:
		TrackerInterface(const KUrl & url);
		virtual ~TrackerInterface();
		
		/// See if a start request succeeded
		bool isStarted() const {return started;}
		
		/// get the tracker url
		KUrl trackerURL() const {return url;}
		
		/// Get the tracker status
		TrackerStatus trackerStatus() const {return status;}
		
		/// Get a string of the current tracker status
		QString trackerStatusString() const;
		
		/**
		* Get the update interval in ms
		* @return interval
		*/
		Uint32 getInterval() const {return interval;}
		
		/// Set the interval
		void setInterval(Uint32 i) {interval = i;}
		
		/// Get the number of seeders
		int getNumSeeders() const {return seeders;}
		
		/// Get the number of leechers
		int getNumLeechers() const {return leechers;}
		
		/// Get the number of times the torrent was downloaded
		int getTotalTimesDownloaded() const {return total_downloaded;}
		
		/// Enable or disable the tracker
		void setEnabled(bool on) {enabled = on;}
		
		/// Is the tracker enabled
		bool isEnabled() const {return enabled;}
		
		/// Get the time in seconds to the next tracker update
		Uint32 timeToNextUpdate() const;
		
		/// Reset the tracker 
		virtual void reset();
		
	protected:
		KUrl url;
		Uint32 interval;
		int seeders;
		int leechers;
		int total_downloaded;
		bool enabled;
		TrackerStatus status;
		QDateTime request_time;
		QString error;
		bool started;
	};

}

#endif // BT_TRACKERINTERFACE_H
