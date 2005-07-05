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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
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

	/**
	 * @author Joris Guisson
	 * @brief Communicates with the tracker
	 * 
	 * Class to communicate with the tracker. This is an abstract class
	 * because some moron taught it was a good idea to have trackers over
	 * UDP. Hence we have two kinds of trackers : HTTP and UDP.
	 *
	 * Once the data comes in, the Tracker should update the TorrentControl object.
	 */
	class Tracker : public QObject
	{
		Q_OBJECT
	public:
		/**
		 * Constructor, sets the TorrentControl object
		 * @param tc The TorrentControl
		 */
		Tracker(TorrentControl* tc);
		virtual ~Tracker();

		/**
		 * Do a request to the tracker.
		 * @param url The path and query
		 */
		virtual void doRequest(const KURL & url) = 0;

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

		/**
		 * Get the TorrentControl object.
		 * @return The TorrentControl
		 */
		TorrentControl* getTC() {return tc;}
	protected:
		TorrentControl* tc;
	
		SHA1Hash info_hash;
		PeerID peer_id;
		Uint16 port;
		Uint32 uploaded,downloaded,left;
		QString event;
	};

}

#endif
