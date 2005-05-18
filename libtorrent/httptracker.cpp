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
#include "log.h"
#include "httptracker.h"
#include "torrentcontrol.h"
#include "bdecoder.h"

namespace bt
{

	HTTPTracker::HTTPTracker(TorrentControl* tc) : Tracker(tc),http(0),cid(0)
	{
		http = new QHttp(this);
		connect(http,SIGNAL(requestFinished(int, bool )),this,SLOT(requestFinished(int, bool )));
	}


	HTTPTracker::~HTTPTracker()
	{}

	void HTTPTracker::doRequest(const KURL & u)
	{
		//if (url.protocol() != "http")
		//	url.setProtocol("http");

		KURL url = u;
		Out() << "Tracker url : " << url << endl;

		QString query = QString("&info_hash=") + info_hash.toURLString();
		url.addQueryItem("peer_id",peer_id.toString());
		url.addQueryItem("port",QString::number(port));
		url.addQueryItem("uploaded",QString::number(uploaded));
		url.addQueryItem("downloaded",QString::number(downloaded));
		url.addQueryItem("left",QString::number(left));

		if (event != QString::null)
			url.addQueryItem("event",event);
		

		Uint16 http_port = url.port();
		if (http_port == 0)
			http_port = 80;

		doRequest(url.host(),url.encodedPathAndQuery() + query,http_port);
	}

	void HTTPTracker::doRequest(const QString & host,const QString & path,Uint16 p)
	{
		QHttpRequestHeader header( "GET",path);
		header.setValue( "Host",host );

		http->setHost(host,p);
		cid = http->request(header);
	}

	void HTTPTracker::dataRecieved(const QByteArray & ba)
	{
		getTC()->trackerResponse(ba);
	}

	void HTTPTracker::requestFinished(int id,bool err)
	{
		if (cid != id)
			return;

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

}
#include "httptracker.moc"
