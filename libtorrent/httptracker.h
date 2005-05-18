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
#ifndef BTHTTPTRACKER_H
#define BTHTTPTRACKER_H

#include <qhttp.h>
#include "tracker.h"

namespace bt
{

	/**
	 * @author Joris Guisson
	 * @brief Communicates with the tracker
	 *
	 * This class uses the HTTP protocol to communicate with the tracker.
	 */
	class HTTPTracker : public Tracker
	{
		Q_OBJECT
	public:
		HTTPTracker(TorrentControl* tc);	
		virtual ~HTTPTracker();

		/**
		 * Do a request to the tracker.
		 * @param url The tracker's url
		 */
		virtual void doRequest(const KURL & url);
		
	private slots:
		void requestFinished(int id,bool err);

	private:
		void dataRecieved(const QByteArray & ba);
		void doRequest(const QString & host,const QString & path,Uint16 p);
		
	private:
		QHttp* http;
		int cid;
	};

}

#endif
