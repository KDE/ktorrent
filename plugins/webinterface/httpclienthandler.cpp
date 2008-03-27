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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include <QSocketNotifier>
#include <qhttp.h> 
#include <util/log.h>
#include <util/mmapfile.h>
#include "httpserver.h"
#include "httpclienthandler.h"
#include "httpresponseheader.h"
#include "phphandler.h"
#include "phpcodegenerator.h"
		
using namespace bt;

namespace kt
{

	HttpClientHandler::HttpClientHandler(HttpServer* srv,int sock) : srv(srv),client(0),read_notifier(0),write_notifier(0),php_response_hdr(200)
	{
		client = new net::Socket(sock,4);
		client->setNonBlocking();
		read_notifier = new QSocketNotifier(sock,QSocketNotifier::Read,this);
		connect(read_notifier,SIGNAL(activated(int)),this,SLOT(readyToRead(int)));
		write_notifier = new QSocketNotifier(sock,QSocketNotifier::Write,this);
		connect(write_notifier,SIGNAL(activated(int)),this,SLOT(sendOutputBuffer(int)));
		write_notifier->setEnabled(false);
		state = WAITING_FOR_REQUEST;
		bytes_read = 0;
		php = 0;
		data.reserve(1024);
		output_buffer.reserve(4096);
		written = 0;
	}


	HttpClientHandler::~HttpClientHandler()
	{
		delete client;
		delete php;
	}

	void HttpClientHandler::readyToRead(int )
	{
		if (state == PROCESSING_PHP)
			return; 

		Uint32 ba = client->bytesAvailable();
		if (ba == 0)
		{
			// other side has closed connection
			client->close();
			closed();
			return;
		}

		if (state == WAITING_FOR_REQUEST)
		{
			Uint32 off = data.size();
			data.resize(data.size() + ba);
			client->recv((Uint8*)data.data() + off,ba);
			
			int end_of_req = data.indexOf("\r\n\r\n");
			if (end_of_req > 0)
			{
				// We have got the header, so lets parse it
				handleRequest(end_of_req + 4);
			}
		}
		else if (state == WAITING_FOR_CONTENT)
		{
			Uint32 ba = client->bytesAvailable();
			if (ba + bytes_read < header.contentLength())
			{
				Uint32 off = data.size();
				data.resize(off + ba);
				client->recv((Uint8*)data.data() + off,ba);
				bytes_read += ba;
			}
			else
			{
				Uint32 left = header.contentLength() - bytes_read;
				Uint32 off = data.size();
				data.resize(off + left);
				client->recv((Uint8*)data.data() + off,left);
				bytes_read += left;
				srv->handlePost(this,header,data);
			
				data.resize(0);
				state = WAITING_FOR_REQUEST;
				if (client->bytesAvailable() > 0)
					readyToRead(client->fd());
			}
		}
	}
		
	void HttpClientHandler::handleRequest(int header_len)
	{
		header = QHttpRequestHeader(data.left(header_len)); // get the header
		data = data.mid(header_len); // move up the data
	//	Out(SYS_WEB|LOG_DEBUG) << "Parsing request : " << header.toString() << endl;
		if (header.method() == "POST")
		{
			if (header.hasContentLength())
			{
				bytes_read = data.size();
				if (bytes_read >= header.contentLength())
				{
					srv->handlePost(this,header,data.left(header.contentLength()));
					data = data.mid(header.contentLength()); // move past content
				}
				else
					state = WAITING_FOR_CONTENT;
			}
		}
		else if (header.method() == "GET")
		{
			srv->handleGet(this,header);
		}
		else
		{
			srv->handleUnsupportedMethod(this);
		}
		
		if (client->bytesAvailable() > 0)
		{
			readyToRead(0);
		}
		else if (data.size() > 0 && state == WAITING_FOR_REQUEST)
		{
			// check if there is another request, if so handle in 
			int eoh = data.indexOf("\r\n\r\n");
			if (eoh > 0)
				handleRequest(eoh + 4);
		}
	}
	
	bool HttpClientHandler::sendFile(HttpResponseHeader & hdr,const QString & full_path)
	{
	//	Out(SYS_WEB|LOG_DEBUG) << "Sending file " << full_path << endl;
		// first look in cache
		MMapFile* c = srv->cacheLookup(full_path);
		
		if (!c)
		{
			// not in cache so load it
			c = new MMapFile();
			if (!c->open(full_path,MMapFile::READ))
			{
				delete c;
				Out(SYS_WEB|LOG_DEBUG) << "Failed to open file " << full_path << endl;
				return false;
			}
			srv->insertIntoCache(full_path,c);
		}
		
		hdr.setValue("Content-Length",QString::number(c->getSize()));
		
	//	Out(SYS_WEB|LOG_DEBUG) << "HTTP header : " << endl;
	//	Out(SYS_WEB|LOG_DEBUG) << hdr.toString() << endl;
				
		output_buffer.append(hdr.toString().toUtf8());
		output_buffer.append(QByteArray((const char*)c->getDataPointer(),c->getSize()));
		sendOutputBuffer();
	//	Out(SYS_WEB|LOG_DEBUG) << "Finished sending " << full_path << " (" << written << " bytes)" << endl;
		return true;
	}
	
#define HTTP_404_ERROR "<html><head><title>404 Not Found</title></head><body>The requested file was not found !</body></html>"
#define HTTP_500_ERROR "<html><head><title>HTTP/1.1 500 Internal Server Error</title></head><body>HTTP/1.1 Internal Server Error<br>%1</body></html>"

	
	void HttpClientHandler::send404(HttpResponseHeader & hdr,const QString & path)
	{
	//	Out(SYS_WEB|LOG_DEBUG) << "Sending 404 " << path << endl;
		QString data = HTTP_404_ERROR;
		hdr.setValue("Content-Length",QString::number(data.length()));

		output_buffer.append(hdr.toString().toUtf8());
		output_buffer.append(data.toUtf8());
		sendOutputBuffer();
	}
	
	void HttpClientHandler::send500(HttpResponseHeader & hdr)
	{
	//	Out(SYS_WEB|LOG_DEBUG) << "Sending 500 " << endl;
		QString data = QString(HTTP_500_ERROR).arg("An internal server error occurred !");
		hdr.setValue("Content-Length",QString::number(data.length()));

		output_buffer.append(hdr.toString().toUtf8());
		output_buffer.append(data.toUtf8());
		sendOutputBuffer();
	}
	
	void HttpClientHandler::sendResponse(const HttpResponseHeader & hdr)
	{
	//	Out(SYS_WEB|LOG_DEBUG) << "Sending response " << hdr.toString() << endl;
		output_buffer.append(hdr.toString().toUtf8());
		sendOutputBuffer();
	}

	void HttpClientHandler::executePHPScript(
			PhpCodeGenerator* php_gen,
			HttpResponseHeader & hdr,
			const QString & php_exe,
			const QString & php_file,
			const QMap<QString,QString> & args)
	{
	//	Out(SYS_WEB|LOG_DEBUG) << "Launching PHP script " << php_file << endl;
		php = new PhpHandler(php_exe,php_gen);
		if (!php->executeScript(php_file,args))
		{
			QByteArray data = QString(HTTP_500_ERROR).arg("Failed to launch PHP executable !").toUtf8();
			hdr.setResponseCode(500);
			
			hdr.setValue("Content-Length",QString::number(data.length()));
			
			output_buffer.append(hdr.toString().toUtf8());
			output_buffer.append(data);
			sendOutputBuffer();
			state = WAITING_FOR_REQUEST;
		}
		else
		{
			php_response_hdr = hdr;
			connect(php,SIGNAL(finished()),this,SLOT(onPHPFinished()));
			state = PROCESSING_PHP;
			read_notifier->setEnabled(false); // disable read notifier while we are executing PHP
		}
	}
	
	void HttpClientHandler::onPHPFinished()
	{
		const QByteArray & output = php->getOutput();
		php_response_hdr.setValue("Content-Length",QString::number(output.length()));
		
		output_buffer.append(php_response_hdr.toString().toUtf8());
		output_buffer.append(output);
		php->deleteLater();
		php = 0;
		state = WAITING_FOR_REQUEST;
		read_notifier->setEnabled(true);
		sendOutputBuffer();
	}
	
	void HttpClientHandler::sendOutputBuffer(int )
	{
		int r = client->send((const Uint8*)output_buffer.data() + written,output_buffer.size() - written);
		if (r == 0)
		{
			// error happened, close the connection
			closed();
		}
		else
		{
			written += r;
			if (written == output_buffer.size())
			{
				// everything sent
				output_buffer.resize(0);
				write_notifier->setEnabled(false);
				written = 0;
			}
			else
			{
				// enable write_notifier, so we can send the rest later
				write_notifier->setEnabled(true);
			}
		}
	}

}

#include "httpclienthandler.moc"
				 
