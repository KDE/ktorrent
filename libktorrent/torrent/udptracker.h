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
	enum Event
	{
		NONE = 0,
		COMPLETED = 1,
		STARTED = 2,
		STOPPED = 3
	};
	
	class UDPTrackerSocket;

	/**
	 * @author Joris Guisson
	 * @brief Communicates with an UDP tracker
	 *
	 * This class is able to communicate with an UDP tracker.
	 * This is an implementation of the protocol described in
	 * http://xbtt.sourceforge.net/udp_tracker_protocol.html
	 */
	class UDPTracker : public Tracker
	{
		Q_OBJECT
	public:
		UDPTracker(const KURL & url,kt::TorrentInterface* tor,const PeerID & id);
		virtual ~UDPTracker();

		virtual void start();
		virtual void stop();
		virtual void completed();
		virtual void manualUpdate();
		virtual Uint32 failureCount() const {return n;}
		

	private slots:
		void onConnTimeout();
		void connectRecieved(Int32 tid,Int64 connection_id);
		void announceRecieved(Int32 tid,const Array<Uint8> & buf);
		void onError(Int32 tid,const QString & error_string);

	private:
		void sendConnect();
		void sendAnnounce();
		bool doRequest();

	private:
		QHostAddress addr;
		Uint16 udp_port;
		Int32 transaction_id;
		Int64 connection_id;

		Uint32 data_read;
		int n;
		QTimer conn_timer;
		Event event;

		static UDPTrackerSocket* socket;
		static Uint32 num_instances;
	};

}

#endif
