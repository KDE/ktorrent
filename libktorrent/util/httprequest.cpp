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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include <qstringlist.h>
#include <torrent/globals.h>
#include "httprequest.h"
#include "array.h"
#include "log.h"


namespace bt 
{

	HTTPRequest::HTTPRequest(const QString & hdr,const QString & payload,const QString & host,Uint16 port,bool verbose) : hdr(hdr),payload(payload),verbose(verbose)
	{
		sock = new KNetwork::KStreamSocket(host,QString::number(port),this,0);
		sock->enableRead(true);
		sock->enableWrite(true);
		sock->setTimeout(30000);
		sock->setBlocking(true);
		connect(sock,SIGNAL(readyRead()),this,SLOT(onReadyRead()));
		connect(sock,SIGNAL(gotError(int)),this,SLOT(onError(int )));
		connect(sock,SIGNAL(timedOut()),this,SLOT(onTimeout()));
	}
	
	
	HTTPRequest::~HTTPRequest()
	{
		sock->close();
		delete sock;
	}
	
	void HTTPRequest::start()
	{
		if (sock->connect())
		{
			payload = payload.replace("$LOCAL_IP",sock->localAddress().nodeName());
			hdr = hdr.replace("$CONTENT_LENGTH",QString::number(payload.length()));
			
			QString req = hdr + payload;
			if (verbose)
			{
				Out() << "Sending " << endl;
				Out() << hdr << payload << endl;
			}
			sock->writeBlock(req.ascii(),req.length());
		}
		else
		{
			error(this,false);
			sock->close();
		}
	}
	
	void HTTPRequest::onReadyRead()
	{
		Uint32 ba = sock->bytesAvailable();
		Array<char> data(ba);
		ba = sock->readBlock(data,ba);
		QString strdata((const char*)data);
		QStringList sl = QStringList::split("\r\n",strdata,false);	
		
		if (verbose)
		{
			Out() << "Got reply : " << endl;
			Out() << strdata << endl;
		}
		
		if (sl.first().contains("HTTP") && sl.first().contains("200"))
		{
			// emit reply OK
			replyOK(this,sl.last());
		}
		else
		{
			// emit reply error
			replyError(this,sl.last());
		}
	}
	
	void HTTPRequest::onError(int)
	{
		Out() << "HTTPRequest error : " << sock->errorString() << endl;
		error(this,false);
		sock->close();
	}
	
	void HTTPRequest::onTimeout()
	{
		Out() << "HTTPRequest timeout" << endl;
		error(this,true);
		sock->close();
	}


}
#include "httprequest.moc"
