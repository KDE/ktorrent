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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
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
#include <netinet/in_systm.h>
#include <netinet/ip.h> 
#include <netinet/tcp.h>
#include <net/socketmonitor.h>
#include "streamsocket.h"
#include "rc4encryptor.h"

using namespace bt;
using namespace net;

namespace mse
{
	
	Uint8 StreamSocket::tos = IPTOS_THROUGHPUT;
	Uint32 StreamSocket::num_connecting = 0;
	Uint32 StreamSocket::max_connecting = 50;

	StreamSocket::StreamSocket() : sock(0),enc(0),monitored(false)
	{
		sock = new BufferedSocket(true);
		sock->setNonBlocking();
		reinserted_data = 0;
		reinserted_data_size = 0;
		reinserted_data_read = 0;
		
	}

	StreamSocket::StreamSocket(int fd) : sock(0),enc(0),monitored(false)
	{
		sock = new BufferedSocket(fd);
		sock->setNonBlocking();
		reinserted_data = 0;
		reinserted_data_size = 0;
		reinserted_data_read = 0;
		sock->setTOS(tos);
	}

	StreamSocket::~StreamSocket()
	{
		// make sure the number of connecting sockets is updated
		if (connecting() && num_connecting > 0)
			num_connecting--;
		
		SocketMonitor::instance().remove(sock);
		delete [] reinserted_data;
		delete enc;
		delete sock;
	}
	
	void StreamSocket::startMonitoring(net::SocketReader* rdr,net::SocketWriter* wrt)
	{
		this->rdr = rdr;
		this->wrt = wrt;
		sock->setReader(this);
		sock->setWriter(this);
		SocketMonitor::instance().add(sock);
		monitored = true;
		if (reinserted_data)
		{
			if (enc)
				enc->decrypt(reinserted_data + reinserted_data_read,
							 reinserted_data_size - reinserted_data_read);
			
			rdr->onDataReady(reinserted_data + reinserted_data_read,
							 reinserted_data_size - reinserted_data_read);
			delete [] reinserted_data;
			reinserted_data = 0;
			reinserted_data_size = 0;
		}
	}
	
		
	Uint32 StreamSocket::sendData(const Uint8* data,Uint32 len)
	{
		if (enc)
		{
			// we need to make sure all data is sent because of the encryption
			Uint32 ds = 0;
			const Uint8* ed = enc->encrypt(data,len);
			while (sock->ok() && ds < len)
			{
				Uint32 ret = sock->send(ed + ds,len - ds);
				ds += ret;
				if (ret == 0)
				{
					Out(SYS_CON|LOG_DEBUG) << "ret = 0" << endl;
				}
			}
			if (ds != len)
				Out() << "ds != len" << endl;
			return ds;
		}
		else
		{
			Uint32 ret = sock->send(data,len);
			if (ret != len)
				Out() << "ret != len" << endl;
			return ret;
		}
	}
		
	Uint32 StreamSocket::readData(Uint8* buf,Uint32 len)
	{
		Uint32 ret2 = 0;
		if (reinserted_data)
		{
			Uint32 tr = reinserted_data_size - reinserted_data_read;
			if (tr < len)
			{
				memcpy(buf,reinserted_data + reinserted_data_read,tr);
				delete [] reinserted_data;
				reinserted_data = 0;
				reinserted_data_size = reinserted_data_read = 0;
				ret2 = tr;
				if (enc)
					enc->decrypt(buf,tr);
			}
			else
			{
				tr = len;
				memcpy(buf,reinserted_data + reinserted_data_read,tr);
				reinserted_data_read += tr;
				if (enc)
					enc->decrypt(buf,tr);
				return tr;
			}
		}
		
		if (len == ret2)
			return ret2;
		
		Uint32 ret = sock->recv(buf + ret2,len - ret2);
		if (ret + ret2 > 0 && enc)
			enc->decrypt(buf,ret + ret2);
		
		return ret;
	}
		
	Uint32 StreamSocket::bytesAvailable() const
	{
		Uint32 ba = sock->bytesAvailable();
		if (reinserted_data_size - reinserted_data_read > 0)
			return  ba + (reinserted_data_size - reinserted_data_read);
		else
			return ba;
	}
	
	void StreamSocket::close()
	{
		sock->close();
	}
	
	bool StreamSocket::connectTo(const QString & ip,Uint16 port)
	{
		// do a safety check
		if (ip.isNull() || ip.length() == 0)
			return false;
		
		// we don't wanna block the current thread so set non blocking
		sock->setNonBlocking();
		if (sock->connectTo(Address(ip,port)))
		{
			sock->setTOS(tos);
			return true;
		}
		else if (connecting())
		{
			num_connecting++;
		}
		
		return false;
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
	
	bool StreamSocket::ok() const
	{
		return sock->ok();
	}

	QString StreamSocket::getRemoteIPAddress() const
	{
		return sock->getPeerName().toString();
	}
	
	bt::Uint16 StreamSocket::getRemotePort() const
	{
		return sock->getPeerName().port();
	}
	
	net::Address StreamSocket::getRemoteAddress() const
	{
		return sock->getPeerName();
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
	
	bool StreamSocket::connecting() const
	{
		return sock->state() == net::Socket::CONNECTING;
	}
	
	void StreamSocket::onDataReady(Uint8* buf,Uint32 size)
	{
		if (enc)
			enc->decrypt(buf,size);
		
		if (rdr)
			rdr->onDataReady(buf,size);
	}
	
	Uint32 StreamSocket::onReadyToWrite(Uint8* data,Uint32 max_to_write)
	{
		if (!wrt)
			return 0;
		
		Uint32 ret = wrt->onReadyToWrite(data,max_to_write);
		if (enc && ret > 0) // do encryption if necessary
			enc->encryptReplace(data,ret);
		
		
		return ret;
	}
	
	bool StreamSocket::hasBytesToWrite() const
	{
		return wrt ? wrt->hasBytesToWrite() : false;
	}
	
	float StreamSocket::getDownloadRate() const
	{
		return sock ? sock->getDownloadRate() : 0.0f;
	}
	
	float StreamSocket::getUploadRate() const
	{
		return sock ? sock->getUploadRate() : 0.0f;
	}
	
	bool StreamSocket::connectSuccesFull() const 
	{
		bool ret = sock->connectSuccesFull();
		if (ret)
			sock->setTOS(tos);
		
		if (num_connecting > 0)
			num_connecting--;
		
		return ret;
	}
	
	void StreamSocket::setGroupIDs(Uint32 up,Uint32 down)
	{
		sock->setGroupID(up,true);
		sock->setGroupID(down,false);
	}
}

#include "streamsocket.moc"
