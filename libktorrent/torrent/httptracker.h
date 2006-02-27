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
#ifndef BTHTTPTRACKER_H
#define BTHTTPTRACKER_H

#include <qtimer.h>
#include "tracker.h"

namespace KIO
{
	class Job;
}

namespace bt
{
	

	/**
	 * @author Joris Guisson
	 * @brief Communicates with the tracker
	 *
	 * This class uses the HTTP protocol to communicate with the tracker.
	 */
	class HTTPTracker : public TrackerBackend
	{
		Q_OBJECT
	public:
		HTTPTracker(Tracker* trk);
		virtual ~HTTPTracker();
		
		virtual void doRequest(const KURL & url);
		virtual void updateData(PeerManager* pman);
		
		
	private slots:
		void onResult(KIO::Job* j);
		void onDataRecieved(KIO::Job* j,const QByteArray & ba);
		void onTimeout();

	private:
		void doRequest(const QString & host,const QString & path,Uint16 p);
		
	private:
		QTimer conn_timer;
		int num_attempts;
		KURL last_url;
		QByteArray data;
		KIO::Job* active_job;
	};

}

#endif
