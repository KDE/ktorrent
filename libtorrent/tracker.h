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
#ifndef BTTRACKER_H
#define BTTRACKER_H

#include <qobject.h>
#include "globals.h"
#include "peerid.h"
#include <libutil/sha1hash.h>

class KURL;

namespace bt
{
	class TorrentControl;
	class PeerManager;

	/**
	 * @author Joris Guisson
	 * @brief Communicates with the tracker
	 * 
	 * Class to communicate with the tracker. This is an abstract class
	 * because there are trackers over UDP and HTTP.
	 *
	 * Once the data comes in, the Tracker should emit a signal.
	 */
	class Tracker : public QObject
	{
		Q_OBJECT
	public:
		/**
		 * Constructor.
		 */
		Tracker();
		virtual ~Tracker();

		/// Get number of seeders.
		Uint32 getNumSeeders() const {return seeders;}

		/// Get number of leechers.
		Uint32 getNumLeechers() const {return leechers;}
		
		/**
		 * Do a request to the tracker.
		 * @param url The path and query
		 */
		virtual void doRequest(const KURL & url) = 0;

		/**
		 * Update all the data. If something is wrong in this function,
		 * an Error should be thrown.
		 * @param tc The TorrentControl
		 * @param pman The PeerManager
		 */
		virtual void updateData(TorrentControl* tc,PeerManager* pman) = 0;
		
		/**
		 * Set all the data needed to do a tracker update 
		 * @param ih 
		 * @param pid 
		 * @param port 
		 * @param uploaded 
		 * @param downloaded 
		 * @param left 
		 * @param event 
		 */
		void setData(const SHA1Hash & ih,const PeerID & pid,Uint16 port,
					 Uint32 uploaded,Uint32 downloaded,Uint32 left,
					 const QString & event);

	signals:
		/**
		 * An error occured during the update.
		 */
		void error();

		/**
		 * Update succeeded data has been recieved.
		 */
		void dataReady();

	protected:
		SHA1Hash info_hash;
		PeerID peer_id;
		Uint16 port;
		Uint32 uploaded,downloaded,left,seeders,leechers;
		QString event;
	};

}

#endif
