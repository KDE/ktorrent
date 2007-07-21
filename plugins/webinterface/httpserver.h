  /***************************************************************************
 *   Copyright (C) 2006 by Diego R. Brogna                                 *
 *   dierbro@gmail.com                                               	   *
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
#ifndef HTTPSERVER_H
#define HTTPSERVER_H
		
#include <qcache.h>
#include <qhttp.h>
#include <qdatetime.h>
#include <qserversocket.h>		
#include <util/ptrmap.h>

class QSocket;

namespace bt
{
	class MMapFile;
}

namespace kt 
{
	class CoreInterface;
	
	/**
	 * @author Diego R. Brogna
	 */
	struct Session
	{
		bool logged_in;
		QTime last_access;
		int sessionId;
	};
	
	struct HeaderField
	{
		bool gzip;
		bool keepAlive;
		int sessionId;
		bool ifModifiedSince;
	};
		
	class PhpHandler;
	class PhpInterface;
	class HttpClientHandler;
	class HttpResponseHeader;
	

	
	class HttpServer : public QServerSocket
	{
		Q_OBJECT
	public:
		HttpServer(CoreInterface *core, int port);
		virtual ~HttpServer();
		
		void newConnection(int s);
		
		void handleGet(HttpClientHandler* hdlr,const QHttpRequestHeader & hdr,bool do_not_check_session = false);
		void handlePost(HttpClientHandler* hdlr,const QHttpRequestHeader & hdr,const QByteArray & data);
		void handleUnsupportedMethod(HttpClientHandler* hdlr);
		bt::MMapFile* cacheLookup(const QString & name);
		void insertIntoCache(const QString & name,bt::MMapFile* file);

	protected slots:
		void slotSocketReadyToRead();
		void slotConnectionClosed();
		
	private:
		bool checkSession(const QHttpRequestHeader & hdr);
		bool checkLogin(const QHttpRequestHeader & hdr,const QByteArray & data);
		void setDefaultResponseHeaders(HttpResponseHeader & hdr,const QString & content_type,bool with_session_info);
		void handleTorrentPost(HttpClientHandler* hdlr,const QHttpRequestHeader & hdr,const QByteArray & data);
		QDateTime parseDate(const QString & str);
		void redirectToLoginPage(HttpClientHandler* hdlr);
		
	private:
		QString rootDir;
		int sessionTTL;
		PhpInterface *php_i;
		Session session;
		bt::PtrMap<QSocket*,HttpClientHandler> clients;
		CoreInterface *core;
		QCache<bt::MMapFile> cache;
	};

	
}
#endif // HTTPSERVER_H
