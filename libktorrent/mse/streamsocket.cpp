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
#include <errno.h>
#include <qsocket.h>
#include <qsocketdevice.h>
#include <util/sha1hash.h>
#include <util/log.h>
#include <torrent/peer.h>
#include <torrent/globals.h>
#include <torrent/authenticatebase.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> 
#include <netinet/tcp.h>
#include "streamsocket.h"
#include "rc4encryptor.h"

using namespace bt;

namespace mse
{
	
	static bool SetTOS(QSocket* s,char type_of_service)
	{
		char c = type_of_service;
		if (setsockopt(s->socketDevice()->socket(),IPPROTO_IP,IP_TOS,&c,sizeof(char)) < 0)
		{
			Out() << QString("Failed to set TOS to %1 : %2")
					.arg((int)type_of_service).arg(strerror(errno)) << endl;
			return false;
		}
		return true;
	}

	StreamSocket::StreamSocket() : sock(0),enc(0)
	{
		sock = new QSocket();
	//	sock->socketDevice()->setReceiveBufferSize(49512);
	//	sock->socketDevice()->setSendBufferSize(49512);
		reinserted_data = 0;
		reinserted_data_size = 0;
		reinserted_data_read = 0;
		connect(sock,SIGNAL(connected()),this,SLOT(onConnected()));
	}

	StreamSocket::StreamSocket(int fd) : sock(0),enc(0)
	{
		sock = new QSocket();
		sock->setSocket(fd);
	//	sock->socketDevice()->setReceiveBufferSize(49512);
	//	sock->socketDevice()->setSendBufferSize(49512);
		reinserted_data = 0;
		reinserted_data_size = 0;
		reinserted_data_read = 0;
		SetTOS(sock,IPTOS_THROUGHPUT);
	}

	StreamSocket::~StreamSocket()
	{
		delete [] reinserted_data;
		delete enc;
		sock->deleteLater();
	}
	
		
	void StreamSocket::sendData(const Uint8* data,Uint32 len)
	{
		if (enc)
			sock->writeBlock((const char*)enc->encrypt(data,len),len);
		else
			sock->writeBlock((const char*)data,len);
	}
		
	Uint32 StreamSocket::readData(Uint8* buf,Uint32 len)
	{	
		Uint32 off = 0;
		if (reinserted_data && reinserted_data_size - reinserted_data_read > 0)
		{
			Uint32 tr = reinserted_data_size - reinserted_data_read;
			if (tr > len)
				tr = len;
			
			memcpy(buf,reinserted_data + reinserted_data_read,tr);
			reinserted_data_read += tr;
			if (reinserted_data_size == reinserted_data_read)
			{
				// the reinserted data has been read, so clear it
				delete [] reinserted_data;
				reinserted_data = 0;
				reinserted_data_size = reinserted_data_read = 0;
			}
			if (enc)
				enc->decrypt(buf,tr);
			
			len -= tr;
			off = tr;
			if (len == 0)
				return tr;
		}
		
		Uint32 ret = sock->readBlock((char*)buf + off,len);
		if (ret > 0 && enc)
			enc->decrypt(buf,ret);
		
		return ret;
	}
		
	Uint32 StreamSocket::bytesAvailable() const
	{
		if (reinserted_data_size - reinserted_data_read > 0)
			return sock->bytesAvailable() + (reinserted_data_size - reinserted_data_read);
		else
			return sock->bytesAvailable();
	}
	
	void StreamSocket::close()
	{
		sock->close();
	}
	
	void StreamSocket::connectTo(const QString & ip,Uint16 port)
	{
		sock->connectToHost(ip,port);
	}
	
	void StreamSocket::onConnected()
	{
		SetTOS(sock,IPTOS_THROUGHPUT);
	}
		
	void StreamSocket::initCrypt(const bt::SHA1Hash & dkey,const bt::SHA1Hash & ekey)
	{
		if (enc)
			delete enc;
		
		enc = new RC4Encryptor(dkey,ekey);
	}
		
	void StreamSocket::disableCrypt()
	{
		delete enc;
		enc = 0;
	}
	
	void StreamSocket::attachPeer(bt::Peer* peer)
	{
		QObject::connect(sock,SIGNAL(connectionClosed()),peer,SLOT(connectionClosed()));
		QObject::connect(sock,SIGNAL(readyRead()),peer,SLOT(readyRead()));
		QObject::connect(sock,SIGNAL(error(int)),peer,SLOT(error(int)));
		QObject::connect(sock,SIGNAL(bytesWritten(int)),peer,SLOT(dataWritten(int )));
	}
	
	void StreamSocket::detachPeer(bt::Peer* peer)
	{
		QObject::disconnect(sock,SIGNAL(connectionClosed()),peer,SLOT(connectionClosed()));
		QObject::disconnect(sock,SIGNAL(readyRead()),peer,SLOT(readyRead()));
		QObject::disconnect(sock,SIGNAL(error(int)),peer,SLOT(error(int)));
		QObject::disconnect(sock,SIGNAL(bytesWritten(int)),peer,SLOT(dataWritten(int)));
	}
	
	void StreamSocket::attachAuthenticate(bt::AuthenticateBase* auth)
	{
		QObject::connect(sock,SIGNAL(readyRead()),auth,SLOT(onReadyRead()));
		QObject::connect(sock,SIGNAL(error(int)),auth,SLOT(onError(int )));
	}
	
	void StreamSocket::detachAuthenticate(bt::AuthenticateBase* auth)
	{
		QObject::disconnect(sock,SIGNAL(readyRead()),auth,SLOT(onReadyRead()));
		QObject::disconnect(sock,SIGNAL(error(int)),auth,SLOT(onError(int )));
	}
	
	void StreamSocket::onConnected(QObject* obj,const char* method)
	{
		QObject::connect(sock,SIGNAL(connected()),obj,method);
	}

	QString StreamSocket::getIPAddress() const
	{
		return sock->peerAddress().toString();
	}
	
	void StreamSocket::setRC4Encryptor(RC4Encryptor* e)
	{
		if (enc)
			delete enc;
		
		enc = e;
	}
	
	void StreamSocket::reinsert(const Uint8* d,Uint32 size)
	{
//		Out() << "Reinsert : " << size << endl;
		Uint32 off = 0;
		if (reinserted_data)
		{
			off = reinserted_data_size;
			reinserted_data = (Uint8*)realloc(reinserted_data,reinserted_data_size + size);
			reinserted_data_size += size;
		}
		else
		{
			reinserted_data = new Uint8[size];
			reinserted_data_size = size;
		}
		memcpy(reinserted_data + off,d,size);
	}
}

#include "streamsocket.moc"
