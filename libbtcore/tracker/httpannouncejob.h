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

#ifndef BT_HTTPANNOUNCEJOB_H
#define BT_HTTPANNOUNCEJOB_H

#include <btcore_export.h>
#include <kio/jobclasses.h>
#include <QHttp>
#include <QBuffer>

namespace bt 
{
	/**
		KIO::Job which uses QHttp to announce to a tracker.
		This was added because KIO doesn't handle the (invalid)http of some trackers in a consistent manner.
	*/
	class BTCORE_EXPORT HTTPAnnounceJob : public KIO::Job
	{
		Q_OBJECT
	public:
		HTTPAnnounceJob(const KUrl & url);
		virtual ~HTTPAnnounceJob();
		
		/**
			Set the proxy to use.
			@param host The proxy hostname or IP
			@param port The port of the proxy
		*/
		void setProxy(const QString & host,int port);
		
		/// Get the announce url
		KUrl announceUrl() const {return url;}
		
		/// Get the reply data
		const QByteArray & replyData() const {return reply_data;}
		
		virtual void start();
		virtual void kill(bool quietly=true);
		
	private slots:
		void requestFinished(int id,bool err);
		void readData(const QHttpResponseHeader & hdr);
		void sendRequest();
		
	private:
		void handleRedirect(const QHttpResponseHeader & hdr);
		
	private:
		KUrl url;
		QHttp* http;
		QByteArray reply_data;
		int get_id;
		
		QString proxy_host;
		int proxy_port;
	};

}

#endif // BT_HTTPANNOUNCEJOB_H
