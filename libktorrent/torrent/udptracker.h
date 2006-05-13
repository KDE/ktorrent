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
#ifndef BTUDPTRACKER_H
#define BTUDPTRACKER_H

#include <kurl.h>
#include <qhostaddress.h>
#include <qvaluelist.h>
#include <qtimer.h>
#include <util/array.h>
#include "tracker.h"
#include "globals.h"
#include "peermanager.h"


namespace bt
{
	class UDPTrackerSocket;

	/**
	 * @author Joris Guisson
	 * @brief Communicates with an UDP tracker
	 *
	 * This class is able to communicate with an UDP tracker.
	 * This is an implementation of the protocol described in
	 * http://xbtt.sourceforge.net/udp_tracker_protocol.html
	 */
	class UDPTracker : public TrackerBackend
	{
		Q_OBJECT
	public:
		UDPTracker(Tracker* trk);
		virtual ~UDPTracker();

		virtual bool doRequest(const KURL & url);
		virtual void updateData(PeerManager* pman);

	private slots:
		void onConnTimeout();
		void connectRecieved(Int32 tid,Int64 connection_id);
		void announceRecieved(Int32 tid,const Array<Uint8> & buf);
		void onError(Int32 tid,const QString & error_string);

	private:
		void sendConnect();
		void sendAnnounce();

	private:
		QHostAddress addr;
		Uint16 udp_port;
		Int32 transaction_id;
		Int64 connection_id;
		KURL old_url;

		Uint32 interval,data_read;
		QValueList<PotentialPeer> ppeers;
		int n;
		QTimer conn_timer;

		static UDPTrackerSocket* socket;
		static Uint32 num_instances;
	};

}

#endif
