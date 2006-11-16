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
#include <kgenericfactory.h>
#include <kglobal.h>

#include <qstring.h>
#include <qmap.h>
#include <qregexp.h>
#include <qserversocket.h>
#include <qsocket.h>
#include <qfile.h>
#include <qcache.h>
#include <qthread.h>
#include "php_handler.h"
#include "php_interface.h"
#include <torrent/peermanager.h>
namespace kt {
	/**
	 * @author Diego R. Brogna
	 */
	
	struct Session{
	bool logged;
	QTime last_access;
	int sessionId;
	};
	
	struct HeaderFiled{
	bool gzip;
	bool keepAlive;
	int sessionId;
	};
	
	class Image{
	public:
		Image(){
			data=0;
		}
		void setName(QString s){name=s;};
		QString getName();
		void *data;
		void setTime(QTime);
		QTime getTime();
	private:
		QString name;
		QTime init_time;
	};
	
	class HttpServer : public QServerSocket
	{
		Q_OBJECT
	public:
		HttpServer(CoreInterface *core, int port);
		~HttpServer();
		void newConnection(int s);
	private:
		QString waitPostData(QSocket* s);
		void parseRequest(QString request);
		void parseHeaderFields(QStringList headerLines);
		void processRequest(QSocket* s);
		void sendHtmlPage(QSocket* s, const char* data);
		void sendRawData(QSocket* s,QString header, QFile *file);

	protected slots:
		void slotSocketReadyToRead();
		void slotConnectionClosed();
	private:
		QString rootDir;
		int sessionTTL;
		PhpHandler *php_h;
		PhpInterface *php_i;
		QCache<Image> imgCache;
		QString requestedFile;
		QMap<QString, QString> requestParams;
		Session session;
		HeaderFiled headerField;
		bool locked;
	};

	class ServerThread : public QThread{
	public:
		ServerThread(CoreInterface *c)
		{
			core=c;
			running=false;
			p=0;
			//server=0;
		}
		~ServerThread()
		{		}
		void stop();		
		void run();
		//bool ok(){return server->ok();}
		int port(){return p;}
		QMutex mutex;
	private:
		bool running;
		CoreInterface *core;
		int p;
		//QMutex mutex;
	};

	
}
#endif // HTTPSERVER_H
