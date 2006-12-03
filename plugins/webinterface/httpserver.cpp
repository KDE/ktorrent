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
#include <qcstring.h>
#include <qdatetime.h>

#include <torrent/queuemanager.h>

#include <interfaces/coreinterface.h>
#include <interfaces/torrentinterface.h>
#include <util/log.h>
#include <net/socketmonitor.h>
#include <qfileinfo.h>
#include <kmdcodec.h>
#include <qsocketnotifier.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <net/portlist.h>

#include <sys/mman.h>
#include "httpserver.h"

#include "webinterfacepluginsettings.h"

#define RAWREAD_BUFF_SIZE	2048

#define HTTP_404_ERROR "<html><head><title>HTTP/1.1 404 Not Found</title></head><body>HTTP/1.1 404 Not Found</body</html>"
#define HTTP_500_ERROR "<html><head><title>HTTP/1.1 500 Internal Server Error</title></head><body>HTTP/1.1 Internal Server Error<br>%1</body</html>"

using namespace bt;

namespace kt{
	
	void ServerThread::run()
	{
		mutex.lock();
		HttpServer *server;
		server=0;
		int i=0, port=WebInterfacePluginSettings::port();
		
		do{
			if(!server)
				delete server;
			server=new HttpServer(core, port+i);
			i++;
		}while(!server->ok() && i<10);

		if(server->ok()){
			if(WebInterfacePluginSettings::forward())
				bt::Globals::instance().getPortList().addNewPort(server->port(),net::TCP,true);
			Out(SYS_WEB|LOG_ALL) << "Web server listen on port "<< server->port() << endl;
		}
		else{
			Out(SYS_WEB|LOG_ALL) << "Cannot bind to port " << port <<" or the 10 following ports. WebInterface plugin cannot be loaded." << endl;
			return;
		}
		p=server->port();
		mutex.unlock();
		running = true;
		while (running)
			usleep(1000);
		running = false;
		delete server;
		return;
	}

	
	void ServerThread::stop()
	{
		running = false;
	}

	HttpServer::HttpServer(CoreInterface *core, int port) : QServerSocket(port, 5){
		php_i=new PhpInterface(core);
		php_h=new PhpHandler(php_i);
		imgCache.setAutoDelete(true);
		QStringList dirList=KGlobal::instance()->dirs()->findDirs("data", "ktorrent/www");
		rootDir=*(dirList.begin());
		Out(SYS_WEB|LOG_DEBUG) << "WWW Root Directory "<< rootDir <<endl;
		session.logged=false;
	}
	HttpServer::~HttpServer()
	{
		delete php_i;
		delete php_h;
	}

	void HttpServer::newConnection(int s){
		QSocket* socket= new QSocket(this);
		connect(socket, SIGNAL(readyRead()), this, SLOT(slotSocketReadyToRead()));
		connect(socket, SIGNAL(delayedCloseFinished()), this, SLOT(slotConnectionClosed()));
		socket->setSocket(s);
		Out(SYS_WEB|LOG_DEBUG) << "connection from "<< socket->peerAddress().toString()  << endl;
	}

	void HttpServer::slotSocketReadyToRead(){
		QString request, data, line;
		unsigned int size=0;
		bool torrentUpload=false, sessionValid=false;
		QSocket* socket = (QSocket*)sender();
		
		while(1)
		{
			line=socket->readLine();
			if(line.isEmpty()){
				if(socket->waitForMore(500)>0)
					continue;
				else
					break;
			}
			if(line=="\r\n")
				break;
			data.append(line);
		}

        	if ( !data.isEmpty() ) {
			QStringList headerLines = QStringList::split("\r\n", data);
			QStringList requestLine = QStringList::split( QRegExp("[ \r\n][ \r\n]*"), headerLines[0]);
			
			if(requestLine[0]=="GET" || requestLine[0]=="POST")
			{
				if(requestLine[1].isEmpty() || !requestLine[1].startsWith("/")){
					socket->close();
					return;
					}
				request=requestLine[1];

				if(requestLine[0]=="POST"){
					request.append('?');
					
					//process post Header
					for(QStringList::Iterator it = headerLines.begin(); it != headerLines.end(); ++it)
					{
						if((*it).contains("Content-Type:") && (*it).contains("multipart/form-data")){
							torrentUpload=true;
							}
						else if((*it).contains("Cookie:")){
							QString l=(*it);
							QStringList tokens = QStringList::split('=', l.remove("Cookie: "));
							if(tokens[0]=="SESSID")
								if(tokens[1].toInt()==session.sessionId)
									sessionValid=true;
						}
						else if((*it).contains("Content-Length:")){
							QStringList token=QStringList::split(":", (*it));
							size=token[1].toInt();
						}
					}
					if(torrentUpload && !sessionValid)
						request.remove('?');	
					else
						request.append(readPostData(socket, size, torrentUpload));

				}
				Out(SYS_WEB| LOG_DEBUG) << "request from "<< socket->peerAddress().toString() << endl;
		
				parseRequest(request);
				
				parseHeaderFields(headerLines);
				
				processRequest(socket);
				

			}
			else 
			{
				Out(SYS_WEB| LOG_DEBUG) << "Sorry method "<< requestLine[0].latin1() <<"not yet supported." << endl;
			}
		}
	
		socket->close();

	}
	
	QString HttpServer::readPostData(QSocket* s, unsigned int size, bool up)
	{
		if(up){
			QStringList header;
			QString line, name;
			while(1)
                        {
                                line=s->readLine();
                                if(line.isEmpty()){
                                        if(s->waitForMore(500)>0)
                                                continue;
                                        else
                                                break;
                                }
				size-=line.length();
                                if(line=="\r\n")
                                        break;
                                header.append(line);
                        }
                        for(QStringList::Iterator it = header.begin(); it != header.end(); ++it)
                        {
                                if((*it).contains("Content-Disposition:") && (*it).contains("filename=")){
                                        QStringList tokens = QStringList::split(';', (*it).remove("Content-Disposition: "));
                                        for(QStringList::Iterator it2 = tokens.begin(); it2 != tokens.end(); ++it2)
                                                        if((*it2).contains("filename")){
                                                                QStringList fileRecord = QStringList::split('=', (*it2));
                                                                name=fileRecord[1].remove("\"").remove("\r\n");
                                                        }
                                }

                        }

			QFile file;
			QStringList dirList=KGlobal::instance()->dirs()->findDirs("tmp", "");
			QDir::setCurrent( *(dirList.begin()) );
			file.setName(name);

			if(file.exists())
				do{
					file.setName(QString("%1-webinterface.torrent").arg(rand()));
				}while(file.exists());

			file.open( IO_WriteOnly);
			
			do{
				file.writeBlock(s->readAll());
			}while(s->waitForMore(500)>0 && file.size()<size);

			file.close();
			
			if(size>0){
				if(file.size()==size)
					return QString("load_torrent=")+KURL::encode_string(QString("file://%1").arg(QFileInfo(file).absFilePath()));
			}
			
			return QString("");
		}
		else{
			QString data;
			do{
			data.append(s->readAll());
			}while(s->waitForMore(500)>0 && data.length()<size);

			if(size>0){
				if(data.length()==size)
					return data;
			}
			
			return QString("");
		}
	}

	void HttpServer::slotConnectionClosed()
	{
		QSocket* socket= (QSocket*)sender();
        	delete socket;
		Out(SYS_WEB| LOG_DEBUG) << "connection_closed" << endl;
	}


	void HttpServer::parseRequest(QString request)
	{
		//remove old data
		requestedFile="";
		requestParams.clear();

		requestedFile=request.left(request.find("?"));
		
		
		request.remove(0,requestedFile.length()+1);
		QStringList tokens = QStringList::split("&",request);
		for ( QStringList::Iterator it = tokens.begin(); it != tokens.end(); ++it ) {
			QStringList req=QStringList::split( '=', *it );
			requestParams[req[0]]=req[1];
			if(req[0]!="password")
				Out(SYS_WEB| LOG_DEBUG) << "Request key [" << req[0].latin1() << "] value [" << req[1].latin1() <<"]" << endl;
		}
	}

	void HttpServer::parseHeaderFields(QStringList headerLines)
	{
		headerField.keepAlive=false;
		headerField.gzip=false;
		headerField.ifModifiedSince=false;
		headerField.sessionId=0;
		
		for ( QStringList::Iterator it = headerLines.begin(); it != headerLines.end(); ++it ) {
			if((*it).contains("Connection:")){
				if((*it).contains("keep-alive"))
					headerField.keepAlive=false;
			}
			else if((*it).contains("Cookie:")){
				QStringList tokens = QStringList::split('=', (*it).remove("Cookie: "));
				if(tokens[0]=="SESSID")
					headerField.sessionId=tokens[1].toInt();
					
			}
			else if((*it).contains("Content-Type:")){
				if((*it).contains("gzip"))
					headerField.gzip=true;
			}
			else if((*it).contains("If-Modified-Since:")){
				headerField.ifModifiedSince=true;
			}		
		}
	}
	void HttpServer::processRequest(QSocket* s)
	{	
		QFile f(rootDir+'/'+WebInterfacePluginSettings::skin()+'/'+requestedFile);
		fprintf(stderr, "%s\n", QString(rootDir+'/'+WebInterfacePluginSettings::skin()+'/'+requestedFile).latin1());
		QFileInfo finfo(f);

		//Logout
		if(requestedFile=="/login.html")
			session.logged=false;

		if(headerField.sessionId==session.sessionId){
			if(session.last_access.secsTo(QTime::currentTime())<WebInterfacePluginSettings::sessionTTL()){
				Out(SYS_WEB| LOG_DEBUG) << "Session valid" << endl;
				session.last_access=QTime::currentTime();
				}
			else{
				Out(SYS_WEB| LOG_DEBUG) << "Session expired" << endl;
				session.logged=false;
			}
		}
		else
			session.logged=false;

		if(!session.logged){
			if(requestParams.contains("username") && requestParams.contains("password")){
				KMD5 context(requestParams["password"].utf8());
				if(requestParams["username"]==WebInterfacePluginSettings::username() && context.hexDigest().data()==WebInterfacePluginSettings::password()){
					session.logged=true;
					Out(SYS_WEB|LOG_ALL) << s->peerAddress().toString() << " logged in"  << endl;
					session.sessionId=rand();
					session.last_access=QTime::currentTime();
					requestParams.remove("password");
				}
				else
					session.logged=false;
			}
		}

		if(!session.logged){
			if(finfo.extension()!="ico" && finfo.extension()!="png" && finfo.extension()!="css" && finfo.exists()){
				requestedFile="login.html";
				Out(SYS_WEB| LOG_DEBUG) << "gone wrong" << endl;
				f.setName(rootDir+'/'+WebInterfacePluginSettings::skin()+'/'+requestedFile);
				finfo.setFile(f);
				session.sessionId=0;
			}
		}

		//execute request
		if(session.logged)
			php_i->exec(requestParams);
		QString header;
		
		if ( !f.open(IO_ReadOnly) || (finfo.extension()!="php" && finfo.extension()!="html" && finfo.extension()!="png" && finfo.extension()!="ico" && finfo.extension()!="css") ){
			QString data;
			header="HTTP/1.1 404 Not Found\r\n";
			header+="Server: ktorrent\r\n";
			header+="Cache-Control: private\r\n";
			header+="Connection: close\r\n";
			header+=QString("Date: ")+QDateTime::currentDateTime(Qt::UTC).toString("ddd, dd MMM yyyy hh:mm:ss UTC\r\n");			
			data=HTTP_404_ERROR;
			header+=QString("Content-Length: %1\r\n\r\n").arg(data.length());
			sendHtmlPage(s, QString(header+data).latin1());
			return;
		}
		
		if(finfo.extension()=="html"){
			QString dataFile;
			dataFile=QString(f.readAll().data());
			header="HTTP/1.1 200 OK\r\n";
			header+="Server: ktorrent\r\n";
			header+="Cache-Control: private\r\n";
			header+="Connection: close\r\n";
			header+=QString("Date: ")+QDateTime::currentDateTime(Qt::UTC).toString("ddd, dd MMM yyyy hh:mm:ss UTC\r\n");
			header+="Content-Type: text/html\r\n";
			header+=QString("Set-Cookie: SESSID=%1\r\n").arg(session.sessionId);
			header+=QString("Content-Length: %1\r\n\r\n").arg(f.size());
			sendHtmlPage(s, QString(header+dataFile).latin1());
		}
		else if(finfo.extension()=="php"){
			QString dataFile;
			dataFile=QString(f.readAll().data());
			dataFile.truncate(f.size());
			if(php_h->executeScript(WebInterfacePluginSettings::phpExecutablePath(), dataFile, requestParams)){
				header="HTTP/1.1 200 OK\r\n";
				header+="Server: ktorrent\r\n";
				header+="Cache-Control: private\r\n";
				header+="Connection: close\r\n";
				header+=QString("Date: ")+QDateTime::currentDateTime(Qt::UTC).toString("ddd, dd MMM yyyy hh:mm:ss UTC\r\n");
				header+="Content-Type: text/html\r\n";
				header+=QString("Set-Cookie: SESSID=%1\r\n").arg(session.sessionId);
				header+=QString("Content-Length: %1\r\n\r\n").arg(php_h->getOutput().length());
				sendHtmlPage(s, QString(header+php_h->getOutput()).latin1());
				}
			else{
				Out(SYS_WEB|LOG_DEBUG) << "PHP executable error" << endl;
				QString data;
				header="HTTP/1.1 500 OK\r\n";
				header+="Server: ktorrent\r\n";
				header+="Cache-Control: private\r\n";
				header+="Connection: close\r\n";
				header+=QString("Date: ")+QDateTime::currentDateTime(Qt::UTC).toString("ddd, dd MMM yyyy hh:mm:ss UTC\r\n");
				data=QString(HTTP_500_ERROR).arg("PHP executable error");
				header+=QString("Content-Length: %1\r\n\r\n").arg(data.length());
				sendHtmlPage(s, QString(header+data).latin1());
				return;

			}
		}
		else if(finfo.extension()=="ico" || finfo.extension()=="png" || finfo.extension()=="css"){
			if(!headerField.ifModifiedSince){
				header="HTTP/1.1 200 OK\r\n";
				header+="Server: ktorrent\r\n";
				header+=QString("Set-Cookie: SESSID=%1\r\n").arg(session.sessionId);
				header+=QString("Date: ")+QDateTime::currentDateTime(Qt::UTC).toString("ddd, dd MMM yyyy hh:mm:ss UTC\r\n");
				header+=QString("Last-Modified: ")+finfo.lastModified().toString("ddd, dd MMM yyyy hh:mm:ss UTC\r\n");
				header+=QString("Expires: ")+QDateTime::currentDateTime(Qt::UTC).addSecs(3600).toString("ddd, dd MMM yyyy hh:mm:ss UTC\r\n");
				header+="Cache-Control: private\r\n";
				header+=QString("Content-Type: image/%1\r\n").arg(finfo.extension());
				header+=QString("Content-Length: %1\r\n\r\n").arg(finfo.size());
				sendRawData(s, header, &f);
			}
			else{
				header="HTTP/1.1 304 Not Modified\r\n";
				header+="Server: ktorrent\r\n";
				header+=QString("Set-Cookie: SESSID=%1\r\n").arg(session.sessionId);
				header+=QString("Date: ")+QDateTime::currentDateTime(Qt::UTC).toString("ddd, dd MMM yyyy hh:mm:ss UTC\r\n");
				header+="Cache-Control: max-age=0\r\n";
				header+=QString("If-Modified-Since: ")+finfo.lastModified().toString("ddd, dd MMM yyyy hh:mm:ss UTC\r\n");
				header+=QString("Content-Type: text/html\r\n");
				header+=QString("Content-Length: 0\r\n\r\n");
				sendHtmlPage(s, QString(header).latin1());
			}
			
		}

		f.close();
	}

	void HttpServer::sendHtmlPage(QSocket* s, const char *data)
	{
		if(!s->isOpen()){
			return;
		}

		QTextStream os(s);
                os.setEncoding( QTextStream::UnicodeUTF8 );
		os << data;
	}
	
	void HttpServer::sendRawData(QSocket* s,QString header, QFile *file)
	{
		if(!s->isOpen()){
			return;
		}

		QTextStream os(s);
                os.setEncoding( QTextStream::UnicodeUTF8 );
		os << header.latin1();
		Image *im;
		im=imgCache.find(file->name(), true);
		if(im==NULL){
			Image *image= new Image();
			image->data=mmap(0, file->size(), PROT_READ, MAP_PRIVATE, file->handle(), 0);
			Out(SYS_WEB|LOG_DEBUG) << file->name() << " mmaped\n"  << endl;
			if(imgCache.insert(file->name(), image)){
				Out(SYS_WEB|LOG_DEBUG) << file->name() << " added in cache\n"  << endl;
				im=imgCache.find(file->name(), true);
			}
			else{
				Out(SYS_WEB|LOG_DEBUG) << file->name() << " not added in cache\n"  << endl;
				void *data;
				unsigned int count=0, r_size;
				data=malloc(RAWREAD_BUFF_SIZE);
				while(file->size() > count){
					memset(data,0,RAWREAD_BUFF_SIZE);
					r_size=file->readBlock((char *)data, RAWREAD_BUFF_SIZE);
					s->writeBlock((const char *)data, r_size);
					s->flush();
					count+=r_size;
				}
				free(data);
				delete image;
				return;
			}

		}
		s->writeBlock((const char *)im->data, file->size());
	}





}
