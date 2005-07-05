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
#include <kurl.h>
#include <libutil/log.h>
#include "httptracker.h"
#include "torrentcontrol.h"
#include "bdecoder.h"

namespace bt
{

	HTTPTracker::HTTPTracker(TorrentControl* tc) : Tracker(tc),http(0),cid(0),num_attempts(-1)
	{
		http = new QHttp(this);
		connect(http,SIGNAL(requestFinished(int, bool )),this,SLOT(requestFinished(int, bool )));
		connect(&conn_timer,SIGNAL(timeout()),this,SLOT(onTimeout()));
	}


	HTTPTracker::~HTTPTracker()
	{}

	void HTTPTracker::doRequest(const KURL & u)
	{
		//if (url.protocol() != "http")
		//	url.setProtocol("http");
		last_url = u;
		KURL url = u;
		Out() << "Doing tracker request to url : " << url << endl;

		QString query = QString("&info_hash=") + info_hash.toURLString();
		url.addQueryItem("peer_id",peer_id.toString());
		url.addQueryItem("port",QString::number(port));
		url.addQueryItem("uploaded",QString::number(uploaded));
		url.addQueryItem("downloaded",QString::number(downloaded));
		url.addQueryItem("left",QString::number(left));
		url.addQueryItem("compact","1");
		url.addQueryItem("numwant","1000");

		if (event != QString::null)
			url.addQueryItem("event",event);
		

		Uint16 http_port = url.port();
		if (http_port == 0)
			http_port = 80;

		doRequest(url.host(),url.encodedPathAndQuery() + query,http_port);
		//Out() << "Request " << url << endl;
	}

	void HTTPTracker::doRequest(const QString & host,const QString & path,Uint16 p)
	{
		QHttpRequestHeader header( "GET",path);
		header.setValue( "Host",host );

		http->setHost(host,p);
		cid = http->request(header);
		if (num_attempts < 0)
		{
			num_attempts = 0;
			conn_timer.start(30 * 1000);
		}
	}

	void HTTPTracker::dataRecieved(const QByteArray & ba)
	{
		getTC()->trackerResponse(ba);
	}

	void HTTPTracker::requestFinished(int id,bool err)
	{
		if (cid != id)
			return;
		
		conn_timer.stop();
		num_attempts = -1;

		if (!err)
		{
			dataRecieved(http->readAll());
		}
		else
		{
			Out() << "Tracker Error : " << http->errorString() << endl;
			getTC()->trackerResponseError();
		}
	}

	void HTTPTracker::onTimeout()
	{
		num_attempts++;
		Out() << "Tracker timeout " << num_attempts << endl; 
		if (num_attempts >= 5)
		{
			conn_timer.stop();
			num_attempts = -1;
			getTC()->trackerResponseError();
		}
		else
		{
			doRequest(last_url);
		}
	}

}
#include "httptracker.moc"
