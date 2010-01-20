/*
    Copyright (C) 2009 by Joris Guisson (joris.guisson@gmail.com)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "httpannouncejob.h"
#include <btversion.h>
#include <KLocalizedString>
#include <util/log.h>
#include <QTimer>

namespace bt
{

	HTTPAnnounceJob::HTTPAnnounceJob(const KUrl& url) : url(url),get_id(0),proxy_port(-1)
	{
		http = new QHttp(this);
		connect(http,SIGNAL(requestFinished(int,bool)),this,SLOT(requestFinished(int,bool)));
		connect(http,SIGNAL(readyRead(QHttpResponseHeader)),this,SLOT(readData(QHttpResponseHeader)));
	}
	
	HTTPAnnounceJob::~HTTPAnnounceJob()
	{
	}

	void HTTPAnnounceJob::start()
	{
		sendRequest();
	}
	
	void HTTPAnnounceJob::sendRequest()
	{
		QHttp::ConnectionMode mode = url.protocol() == "https" ? QHttp::ConnectionModeHttps : QHttp::ConnectionModeHttp;
		quint16 port = url.port() < 0 ? 0 : url.port();
		quint16 default_port = mode == QHttp::ConnectionModeHttps ? 443 : 80;
		http->setHost(url.host(),mode,port);
		if (!proxy_host.isEmpty() && proxy_port > 0)
			http->setProxy(proxy_host,proxy_port);
		
		QHttpRequestHeader hdr("GET",url.encodedPathAndQuery(),1,1);
		hdr.setValue("User-Agent",bt::GetVersionString());
		hdr.setValue("Connection","close");
		hdr.setValue("Host",QString("%1:%2").arg(url.host()).arg(url.port(default_port)));
		get_id = http->request(hdr);
		Out(SYS_TRK|LOG_DEBUG) << "Request sent" << endl;
	}
	
	void HTTPAnnounceJob::kill(bool quietly)
	{
		http->abort();
		if (!quietly)
			emitResult();
	}

	void HTTPAnnounceJob::requestFinished(int id, bool err)
	{
		if (get_id != id)
			return;
		
		if (err)
		{
			setErrorText(http->errorString());
			emitResult();
		}
		else
		{
			switch (http->lastResponse().statusCode())
			{
				case 300:
				case 301:
				case 302:
				case 303:
				case 307:
					handleRedirect(http->lastResponse());
					break;
				case 403:
					setError(KIO::ERR_ACCESS_DENIED);
					emitResult();
					break;
				case 404:
					setError(KIO::ERR_DOES_NOT_EXIST);
					emitResult();
					break;
				case 500:
					setError(KIO::ERR_INTERNAL_SERVER);
					emitResult();
					break;
				default:
					emitResult();
					break;
			}
		}
	}
		
	void HTTPAnnounceJob::readData(const QHttpResponseHeader& hdr)
	{
		const int MAX_REPLY_SIZE = 1024 * 1024;
		
		int ba = http->bytesAvailable();
		int current_size = reply_data.size();
		if (current_size + ba > MAX_REPLY_SIZE)
		{
			// If the reply is larger then a mega byte, the server
			// has probably gone bonkers
			abort();
			Out(SYS_TRK|LOG_DEBUG) << "Tracker sending back to much data in announce reply, aborting ..." << endl;
		}
		else
		{
			// enlarge reply data and read data to it
			reply_data.resize(current_size + ba);
			http->read(reply_data.data() + current_size,ba);
		}
	}
	
	void HTTPAnnounceJob::setProxy(const QString& host, int port)
	{
		proxy_host = host;
		proxy_port = port;
	}


	void HTTPAnnounceJob::handleRedirect(const QHttpResponseHeader& hdr)
	{
		if (!hdr.hasKey("Location"))
		{
			setErrorText(i18n("Redirect without a redirect location"));
			emitResult();
		}
		else
		{
			reply_data.clear();
			url = hdr.value("Location");
			Out(SYS_TRK|LOG_DEBUG) << "Redirected to " << hdr.value("Location") << endl;
			sendRequest();
		}
	}
}