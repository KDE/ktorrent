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
#include <qhttp.h>
#include "globals.h"

namespace bt
{

	/**
	 * @author Joris Guisson
	 * @brief Communicates with the tracker
	 * 
	 * This classes uses the HTTP protocol to communicate with the tracker.
	 * It doesn't do jack shit with the data it recieves, it just sends out
	 * a signal.
	 */
	class Tracker : public QObject
	{
		Q_OBJECT
		QHttp* http;
		int cid;
	public:
		Tracker();
		virtual ~Tracker();

	public slots:
		/**
		 * Do a HTTP Get request to the tracker.
		 * @param host The tracker's hostname
		 * @param path The path and query 
		 * @param port The port
		 */
		void doRequest(const QString & host,const QString & path,Uint16 port = 80);
	private slots:
		void requestFinished(int id,bool err);
		
	signals:
		/**
		 * Emitted when the HTTP GET request failed.
		 */
		void requestError();
		
		/**
		 * A response was recieved.
		 * @param data The data
		 */
		void response(const QByteArray & data);
	};

};

#endif
