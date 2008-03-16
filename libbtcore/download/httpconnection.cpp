/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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

#include <QtAlgorithms>
#include <kurl.h>
#include <net/socketmonitor.h>
#include <util/log.h>
#include "httpconnection.h"
#include "btversion.h"

namespace bt
{

	HttpConnection::HttpConnection() : sock(0),state(IDLE),mutex(QMutex::Recursive)
	{
	}


	HttpConnection::~HttpConnection()
	{
		if (sock)
		{
			net::SocketMonitor::instance().remove(sock);
			delete sock;
		}
		
		qDeleteAll(requests);
	}
	
	bool HttpConnection::ok() const 
	{
		QMutexLocker locker(&mutex);
		return state != ERROR;
	}
		
	bool HttpConnection::connected() const 
	{
		QMutexLocker locker(&mutex);
		return state == ACTIVE;
	}
	
	bool HttpConnection::closed() const
	{
		QMutexLocker locker(&mutex);
		return state == CLOSED || (sock && !sock->ok());
	}
	
	void HttpConnection::connectTo(const KUrl & url)
	{
		KNetwork::KResolver::resolveAsync(this, SLOT(hostResolved(KNetwork::KResolverResults)), 
										  url.host(), QString::number(url.port() <= 0 ? 80 : url.port()));
		state = RESOLVING;
	}

	void HttpConnection::onDataReady(Uint8* buf,Uint32 size)
	{
		QMutexLocker locker(&mutex);
		
		if (state != ERROR && requests.count() > 0)
		{
			if (size == 0)
			{
				 // connection closed
				state = CLOSED;
			}
			else
			{
				HttpGet* g = requests.front();
				if (!g->onDataReady(buf,size))
				{
					state = ERROR;
				}
			}
		}
	}
	
	Uint32 HttpConnection::onReadyToWrite(Uint8* data,Uint32 max_to_write)
	{
		QMutexLocker locker(&mutex);
		if (state == CONNECTING)
		{
			if (sock->connectSuccesFull())
			{
				//Out(SYS_CON|LOG_DEBUG) << "HttpConnection: connected "  << endl;
				state = ACTIVE;
			}
			else
			{
				Out(SYS_CON|LOG_IMPORTANT) << "HttpConnection: failed to connect to webseed "  << endl;
				state = ERROR;
			}
		}
		else if (state == ACTIVE)
		{
			HttpGet* g = requests.front();
			if (g->request_sent)
				return 0;
			
			Uint32 len = g->buffer.size() - g->bytes_sent;
			if (len > max_to_write)
				len = max_to_write;
			
			memcpy(data,g->buffer.data() + g->bytes_sent,len);
			g->bytes_sent += len;
			if (len == g->buffer.size())
			{
				g->buffer.clear();
				g->request_sent = true;
			}
			return len;
		}
		
		return 0;
	}
	
	bool HttpConnection::hasBytesToWrite() const
	{
		QMutexLocker locker(&mutex);
		if (state == CONNECTING)
			return true;
		
		if (state == ERROR || requests.count() == 0)
			return false;
		
		HttpGet* g = requests.front();
		return !g->request_sent;
	}

	void HttpConnection::hostResolved(KNetwork::KResolverResults res)
	{
		if (res.count() > 0)
		{
			KNetwork::KInetSocketAddress addr = res.front().address();
			sock = new net::BufferedSocket(true,addr.ipVersion());
			sock->setNonBlocking();
			sock->setReader(this);
			sock->setWriter(this);
			
			if (sock->connectTo(addr))
			{
				state = ACTIVE;
				net::SocketMonitor::instance().add(sock);
				net::SocketMonitor::instance().signalPacketReady();
			}
			else if (sock->state() == net::Socket::CONNECTING)
			{
				state = CONNECTING;
				net::SocketMonitor::instance().add(sock);
				net::SocketMonitor::instance().signalPacketReady();
			}
			else 
			{
				Out(SYS_CON|LOG_IMPORTANT) << "HttpConnection: failed to connect to webseed" << endl;
				state = ERROR;
			}
		}
		else
		{
			Out(SYS_CON|LOG_IMPORTANT) << "HttpConnection: failed to resolve hostname of webseed" << endl;
			state = ERROR;
		}
	}
	
	bool HttpConnection::get(const QString & host,const QString & path,bt::Uint64 start,bt::Uint64 len)
	{
		QMutexLocker locker(&mutex);
		if (state == ERROR)
			return false;
			
		HttpGet* g = new HttpGet(host,path,start,len);
		requests.append(g);
		net::SocketMonitor::instance().signalPacketReady();
		return true;
	}
	
	bool HttpConnection::getData(QByteArray & data)
	{
		QMutexLocker locker(&mutex);
		if (requests.count() == 0)
			return false;
		
		HttpGet* g = requests.front();
		if (g->piece_data.size() == 0)
		{
			if (!g->request_sent)
				net::SocketMonitor::instance().signalPacketReady();
			return false;
		}
		
		data = g->piece_data;
		g->piece_data.clear();
		
		// if all the data has been received and passed on to something else
		// remove the current request from the queue
		if (g->piece_data.size() == 0 && g->finished())
		{
			delete g;
			requests.pop_front();
			if (requests.size() > 0)
				net::SocketMonitor::instance().signalPacketReady();
		}
		
		return true;
	}
	
	float HttpConnection::getDownloadRate() const
	{
		QMutexLocker locker(&mutex);
		if (sock)
			return sock->getDownloadRate();
		else
			return 0;
	}
	
	////////////////////////////////////////////
	
	HttpConnection::HttpGet::HttpGet(const QString & host,const QString & path,bt::Uint64 start,bt::Uint64 len) : path(path),start(start),len(len),data_received(0),bytes_sent(0),response_header_received(false),request_sent(false)
	{
		QHttpRequestHeader request("GET",path);
		request.setValue("Connection","Keep-Alive");
		request.setValue("Range",QString("bytes=%1-%2").arg(start).arg(start + len - 1));
		request.setValue("User-Agent",bt::GetVersionString());
		request.setValue("Host",host);
		buffer = request.toString().toLocal8Bit();
	//	Out(SYS_CON|LOG_DEBUG) << "HttpConnection: sending http request:" << endl;
	//	Out(SYS_CON|LOG_DEBUG) << request.toString() << endl;
	}
	
	HttpConnection::HttpGet::~HttpGet()
	{}
	
	bool HttpConnection::HttpGet::onDataReady(Uint8* buf,Uint32 size)
	{
		if (!response_header_received)
		{
			// append the data
			buffer.append(QByteArray((char*)buf,size));
			// look for the end of the header 
			int idx = buffer.indexOf("\r\n\r\n");
			if (idx == -1) // haven't got the full header yet
				return true; 
			
			response_header_received = true;
			QHttpResponseHeader hdr(QString::fromLocal8Bit(buffer.mid(0,idx + 4)));
			
		//	Out(SYS_CON|LOG_DEBUG) << "HttpConnection: http reply header received" << endl;
		//	Out(SYS_CON|LOG_DEBUG) << hdr.toString() << endl;
			if (! (hdr.statusCode() == 200 || hdr.statusCode() == 206))
			{
				return false;
			}
			
			if (buffer.size() - (idx + 4) > 0)
			{
				// more data then the header has arrived so append it to piece_data
				data_received += buffer.size() - (idx + 4);
				piece_data.append(buffer.mid(idx + 4));
			}
		}
		else
		{
			// append the data to the list
			data_received += size;
			piece_data.append(QByteArray((char*)buf,size));
		}
		return true;
	}
}

#include "httpconnection.moc"
