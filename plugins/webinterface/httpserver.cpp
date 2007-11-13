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
#include <qtimer.h>
#include <qcstring.h>
#include <qdatetime.h>
#include <kapplication.h>
#include <kgenericfactory.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kmdcodec.h>
#include <ktempfile.h>

#include <qfileinfo.h>
#include <qsocket.h>
#include <qstringlist.h>

#include <interfaces/coreinterface.h>
#include <interfaces/torrentinterface.h>

#include <util/log.h>		
#include <util/fileops.h>
#include <util/functions.h>
#include <util/mmapfile.h>
#include "ktversion.h"
#include "httpserver.h"
#include "httpclienthandler.h"
#include "httpresponseheader.h"
#include "php_handler.h"
#include "php_interface.h"
#include "webinterfacepluginsettings.h"

using namespace bt;

namespace kt
{
	
	

	HttpServer::HttpServer(CoreInterface *core, int port) : QServerSocket(port, 5),core(core),cache(10,23)
	{
		php_i = new PhpInterface(core);
		clients.setAutoDelete(true);
		
		QStringList dirList = KGlobal::instance()->dirs()->findDirs("data", "ktorrent/www");
		rootDir = *(dirList.begin());
		Out(SYS_WEB|LOG_DEBUG) << "WWW Root Directory "<< rootDir <<endl;
		session.logged_in = false;
		cache.setAutoDelete(true);
	}
	
	HttpServer::~HttpServer()
	{
		delete php_i;
	}

	void HttpServer::newConnection(int s)
	{
		QSocket* socket = new QSocket(this);
		socket->setSocket(s);
	
		connect(socket, SIGNAL(readyRead()), this, SLOT(slotSocketReadyToRead()));
		connect(socket, SIGNAL(delayedCloseFinished()), this, SLOT(slotConnectionClosed()));
		connect(socket, SIGNAL(connectionClosed()), this, SLOT(slotConnectionClosed()));
	
		HttpClientHandler* handler = new HttpClientHandler(this,socket);
		clients.insert(socket,handler);
		Out(SYS_WEB|LOG_NOTICE) << "connection from "<< socket->peerAddress().toString()  << endl;
	}
	

	void HttpServer::slotSocketReadyToRead()
	{
		QSocket* client = (QSocket*)sender();
		HttpClientHandler* handler = clients.find(client);
		if (!handler)
		{
			client->deleteLater();
			return;
		}
		
		handler->readyToRead();
	}
	
	static int DecodeEscapedChar(QString & password,int idx)
	{
		QChar a = password[idx + 1].lower();
		QChar b = password[idx + 2].lower();
		if (!a.isNumber() && !(a.latin1() >= 'a' && a.latin1() <= 'f'))
			return idx + 2; // not a valid hex digit
		
		if (!b.isNumber() && !(b.latin1() >= 'a' && b.latin1() <= 'f'))
			return idx + 2; // not a valid hex digit
		
		// calculate high and low nibble
		Uint8 h = (a.latin1() - (a.isNumber() ? '0' : 'a')) << 4;
		Uint8 l = (b.latin1() - (b.isNumber() ? '0' : 'a'));
		char r = (char) h | l; // combine them and cast to a char
		password.replace(idx,3,r);
		return idx + 1;
	}
	
	bool HttpServer::checkLogin(const QHttpRequestHeader & hdr,const QByteArray & data)
	{
		if (hdr.contentType() != "application/x-www-form-urlencoded")
			return false;
		
		QString username;
		QString password;
		QStringList params = QStringList::split("&",QString(data));
		for (QStringList::iterator i = params.begin();i != params.end();i++)
		{
			QString t = *i;
			if (t.section("=",0,0) == "username")
				username = t.section("=",1,1);
			else if (t.section("=",0,0) == "password")
				password = t.section("=",1,1);
			
			// check for passwords with url encoded stuff in them and decode them if necessary
			int idx = 0;
			while ((idx = password.find('%',idx)) > 0)
			{
				if (idx + 2 < password.length())
				{
					idx = DecodeEscapedChar(password,idx);
				}
				else
					break;
			}
		}

		if (!username.isNull() && !password.isNull())
		{
			KMD5 context(password.utf8());

			if(username == WebInterfacePluginSettings::username() && 
				context.hexDigest().data() == WebInterfacePluginSettings::password())
			{
				session.logged_in = true;
				session.sessionId=rand();
				session.last_access=QTime::currentTime();
				Out(SYS_WEB|LOG_NOTICE) << "Webgui login succesfull !" << endl;
				return true;
			}
		}

		return false;
	}
	
	bool HttpServer::checkSession(const QHttpRequestHeader & hdr)
	{
		// check session in cookie
		int session_id = 0;
		if (hdr.hasKey("Cookie"))
		{
			QString cookie = hdr.value("Cookie");
			int idx = cookie.find("KT_SESSID=");
			if (idx == -1)
				return false;
			
			QString number;
			idx += QString("KT_SESSID=").length();
			while (idx < cookie.length())
			{
				if (cookie[idx] >= '0' && cookie[idx] <= '9')
					number += cookie[idx];
				else
					break;
				
				idx++;
			}
					
			session_id = number.toInt();
		}
		
	
		if (session_id == session.sessionId)
		{
			// check if the session hasn't expired yet
			if(session.last_access.secsTo(QTime::currentTime())<WebInterfacePluginSettings::sessionTTL())
			{
				session.last_access=QTime::currentTime();
			}
			else
			{
				return false;
			}
		}
		else
			return false;

		return true;
	}
	
	static QString ExtensionToContentType(const QString & ext)
	{
		if (ext == "html")
			return "text/html";
		else if (ext == "css")
			return "text/css";
		else if (ext == "js")
			return "text/javascript";
		else if (ext == "gif" || ext == "png" || ext == "ico")
			return "image/" + ext;
		else
			return QString::null;
	}
	
	// HTTP needs non translated dates
	static QString days[] = {
		"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
	};
	
	static QString months[] = {
		"Jan","Feb","Mar","Apr",
		"May","Jun","Jul","Aug",
		"Sep","Oct","Nov","Dec"
	};
	
	static QString DateTimeToString(const QDateTime & now,bool cookie)
	{
		if (!cookie)
			return now.toString("%1, dd %2 yyyy hh:mm:ss UTC")
				.arg(days[now.date().dayOfWeek() - 1])
				.arg(months[now.date().month() - 1]);
		else
			return now.toString("%1, dd-%2-yyyy hh:mm:ss GMT")
				.arg(days[now.date().dayOfWeek() - 1])
				.arg(months[now.date().month() - 1]);
	}
		
	void HttpServer::setDefaultResponseHeaders(HttpResponseHeader & hdr,const QString & content_type,bool with_session_info)
	{
		hdr.setValue("Server","KTorrent/" KT_VERSION_MACRO);
		hdr.setValue("Date",DateTimeToString(QDateTime::currentDateTime(Qt::UTC),false));
		hdr.setValue("Content-Type",content_type);
		hdr.setValue("Connection","keep-alive");
		if (with_session_info && session.sessionId && session.logged_in)
		{
			hdr.setValue("Set-Cookie",QString("KT_SESSID=%1").arg(session.sessionId));
		}
	}
	
	void HttpServer::redirectToLoginPage(HttpClientHandler* hdlr)
	{
		HttpResponseHeader rhdr(301);
		setDefaultResponseHeaders(rhdr,"text/html",false);
		rhdr.setValue("Location","/login.html");
		QString path = rootDir + bt::DirSeparator() + WebInterfacePluginSettings::skin() + "/login.html";
		if (!hdlr->sendFile(rhdr,path))
		{
			HttpResponseHeader nhdr(404);
			setDefaultResponseHeaders(nhdr,"text/html",false);
			hdlr->send404(nhdr,path);
		}
		Out(SYS_WEB|LOG_NOTICE) << "Redirecting to /login.html" << endl;
	}
	
	void HttpServer::handleGet(HttpClientHandler* hdlr,const QHttpRequestHeader & hdr,bool do_not_check_session)
	{
		QString file = hdr.path();
		if (file == "/")
			file = "/login.html";
		
		//Out(SYS_WEB|LOG_DEBUG) << "GET " << hdr.path() << endl;
		
		KURL url;
		url.setEncodedPathAndQuery(file);
		
		QString path = rootDir + bt::DirSeparator() + WebInterfacePluginSettings::skin() + url.path();
		// first check if the file exists (if not send 404)
		if (!bt::Exists(path))
		{
			HttpResponseHeader rhdr(404);
			setDefaultResponseHeaders(rhdr,"text/html",false);
			hdlr->send404(rhdr,path);
			return;
		}
		
		QFileInfo fi(path);
		QString ext = fi.extension();
		
		// if it is the login page send that
		if (file == "/login.html" || file == "/")
		{
			session.logged_in = false;
			ext = "html";
			path = rootDir + bt::DirSeparator() + WebInterfacePluginSettings::skin() + "/login.html"; 
		}
		else if (!session.logged_in && (ext == "html" || ext == "php"))
		{
			// for any html or php page, a login is necessary
			redirectToLoginPage(hdlr);
			return;
		}
		else if (session.logged_in && !do_not_check_session && (ext == "html" || ext == "php"))
		{
			// if we are logged in and it's a html or php page, check the session id
			if (!checkSession(hdr))
			{
				session.logged_in = false;
				// redirect to login page
				redirectToLoginPage(hdlr);
				return;
			}
		}
		
		if (ext == "html")
		{
			HttpResponseHeader rhdr(200);
			setDefaultResponseHeaders(rhdr,"text/html",true);
			if (path.endsWith("login.html"))
			{
				// clear cookie in case of login page
				QDateTime dt = QDateTime::currentDateTime().addDays(-1);
				QString cookie = QString("KT_SESSID=666; expires=%1 +0000").arg(DateTimeToString(dt,true));
				rhdr.setValue("Set-Cookie",cookie);
			}
			
			if (!hdlr->sendFile(rhdr,path))
			{
				HttpResponseHeader nhdr(404);
				setDefaultResponseHeaders(nhdr,"text/html",false);
				hdlr->send404(nhdr,path);
			}
		}
		else if (ext == "css" || ext == "js" || ext == "png" || ext == "ico" || ext == "gif" || ext == "jpg")
		{
			if (hdr.hasKey("If-Modified-Since"))
			{
				QDateTime dt = parseDate(hdr.value("If-Modified-Since"));
				if (dt.isValid() && dt < fi.lastModified())
				{	
					HttpResponseHeader rhdr(304);
					setDefaultResponseHeaders(rhdr,"text/html",true);
					rhdr.setValue("Cache-Control","max-age=0");
					rhdr.setValue("Last-Modified",DateTimeToString(fi.lastModified(),false));
					rhdr.setValue("Expires",DateTimeToString(QDateTime::currentDateTime(Qt::UTC).addSecs(3600),false));
					hdlr->sendResponse(rhdr);
					return;
				}
			}
			
			
			HttpResponseHeader rhdr(200);
			setDefaultResponseHeaders(rhdr,ExtensionToContentType(ext),true);
			rhdr.setValue("Last-Modified",DateTimeToString(fi.lastModified(),false));
			rhdr.setValue("Expires",DateTimeToString(QDateTime::currentDateTime(Qt::UTC).addSecs(3600),false));
			rhdr.setValue("Cache-Control","private");
			if (!hdlr->sendFile(rhdr,path))
			{
				HttpResponseHeader nhdr(404);
				setDefaultResponseHeaders(nhdr,"text/html",false);
				hdlr->send404(nhdr,path);
			}
		}
		else if (ext == "php")
		{
			bool redirect = false;
			bool shutdown = false;
			if (url.queryItems().count() > 0 && session.logged_in)
				redirect = php_i->exec(url,shutdown);
			
			if (shutdown)
			{
				// first send back login page
				redirectToLoginPage(hdlr);
				QTimer::singleShot(1000,kapp,SLOT(quit()));
			}
			else if (redirect)
			{
				HttpResponseHeader rhdr(301);
				setDefaultResponseHeaders(rhdr,"text/html",true);
				rhdr.setValue("Location",url.encodedPathAndQuery());
				
				hdlr->executePHPScript(php_i,rhdr,WebInterfacePluginSettings::phpExecutablePath(),
									   path,url.queryItems());
			}
			else
			{
				HttpResponseHeader rhdr(200);
				setDefaultResponseHeaders(rhdr,"text/html",true);
			
				hdlr->executePHPScript(php_i,rhdr,WebInterfacePluginSettings::phpExecutablePath(),
								   path,url.queryItems());
			}
		}
		else
		{
			HttpResponseHeader rhdr(404);
			setDefaultResponseHeaders(rhdr,"text/html",false);
			hdlr->send404(rhdr,path);
		}
	}
	
	void HttpServer::handlePost(HttpClientHandler* hdlr,const QHttpRequestHeader & hdr,const QByteArray & data)
	{
		// this is either a file or a login
		if (hdr.value("Content-Type").startsWith("multipart/form-data"))
		{
			handleTorrentPost(hdlr,hdr,data);
		}
		else if (!checkLogin(hdr,data))
		{
			QHttpRequestHeader tmp = hdr;
			tmp.setRequest("GET","/login.html",1,1);
			handleGet(hdlr,tmp);
		}
		else
		{
			handleGet(hdlr,hdr,true);
		}
	}
	
	void HttpServer::handleTorrentPost(HttpClientHandler* hdlr,const QHttpRequestHeader & hdr,const QByteArray & data)
	{
		const char* ptr = data.data();
		Uint32 len = data.size();
		int pos = QString(data).find("\r\n\r\n");
		
		if (pos == -1 || pos + 4 >= len || ptr[pos + 4] != 'd')
		{
			HttpResponseHeader rhdr(500);
			setDefaultResponseHeaders(rhdr,"text/html",false);
			hdlr->send500(rhdr);
			return;
		}
		
		// save torrent to a temporary file
		KTempFile tmp_file(locateLocal("tmp", "ktwebgui-"), ".torrent");
		QDataStream* out = tmp_file.dataStream();
		if (!out)
		{
			HttpResponseHeader rhdr(500);
			setDefaultResponseHeaders(rhdr,"text/html",false);
			hdlr->send500(rhdr);
			return;
		}
		
		out->writeRawBytes(ptr + (pos + 4),len - (pos + 4));
		tmp_file.sync();
		tmp_file.setAutoDelete(true);
		
		Out(SYS_WEB|LOG_NOTICE) << "Loading file " << tmp_file.name() << endl;
		core->loadSilently(KURL::fromPathOrURL(tmp_file.name()));
		
		handleGet(hdlr,hdr);
	}
	
	void HttpServer::handleUnsupportedMethod(HttpClientHandler* hdlr)
	{
		HttpResponseHeader rhdr(500);
		setDefaultResponseHeaders(rhdr,"text/html",false);
		hdlr->send500(rhdr);
	}
	
	void HttpServer::slotConnectionClosed()
	{
		QSocket* socket= (QSocket*)sender();
		clients.erase(socket);
	}

	QDateTime HttpServer::parseDate(const QString & str)
	{
		/*
		Potential date formats :
		    Sun, 06 Nov 1994 08:49:37 GMT  ; RFC 822, updated by RFC 1123
      		Sunday, 06-Nov-94 08:49:37 GMT ; RFC 850, obsoleted by RFC 1036
      		Sun Nov  6 08:49:37 1994       ; ANSI C's asctime() format
		*/
		QStringList sl = QStringList::split(" ",str);
		if (sl.count() == 6)
		{
			// RFC 1123 format
			QDate d;
			QString month = sl[2];
			int m = -1;
			for (int i = 1;i <= 12 && m < 0;i++)
				if (QDate::shortMonthName(i) == month)
					m = i;
			
			d.setYMD(sl[3].toInt(),m,sl[1].toInt());
			
			QTime t = QTime::fromString(sl[4],Qt::ISODate);
			return QDateTime(d,t);
		}
		else if (sl.count() == 4)
		{
			//  RFC 1036
			QStringList dl = QStringList::split("-",sl[1]);
			if (dl.count() != 3)
				return QDateTime();
			
			QDate d;
			QString month = dl[1];
			int m = -1;
			for (int i = 1;i <= 12 && m < 0;i++)
				if (QDate::shortMonthName(i) == month)
					m = i;
			
			d.setYMD(2000 + dl[2].toInt(),m,dl[0].toInt());
			
			QTime t = QTime::fromString(sl[2],Qt::ISODate);
			return QDateTime(d,t);
		}
		else if (sl.count() == 5)
		{
			// ANSI C
			QDate d;
			QString month = sl[1];
			int m = -1;
			for (int i = 1;i <= 12 && m < 0;i++)
				if (QDate::shortMonthName(i) == month)
					m = i;
			
			d.setYMD(sl[4].toInt(),m,sl[2].toInt());
			
			QTime t = QTime::fromString(sl[3],Qt::ISODate);
			return QDateTime(d,t);
		}
		else
			return QDateTime();
	}
	
	bt::MMapFile* HttpServer::cacheLookup(const QString & name)
	{
		return cache.find(name);
	}
	
	void HttpServer::insertIntoCache(const QString & name,bt::MMapFile* file)
	{
		cache.insert(name,file);
	}

}

#include "httpserver.moc"
