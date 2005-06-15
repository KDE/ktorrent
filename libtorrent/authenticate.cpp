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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "log.h"
#include "authenticate.h"


namespace bt
{

	Authenticate::Authenticate(QSocket* sock,const SHA1Hash & info_hash,const PeerID & peer_id) 
	: sock(sock),info_hash(info_hash),our_peer_id(peer_id),done(false)
	{
		connect(sock,SIGNAL(connected()),this,SLOT(connected()));
		connect(sock,SIGNAL(readyRead()),this,SLOT(readyRead()));
		connect(sock,SIGNAL(error(int)),this,SLOT(error(int )));
		sendHandshake();
		connect(&timer,SIGNAL(timeout()),this,SLOT(onTimeout()));
		timer.start(10000,true);
		host = sock->peerAddress().toString();
	}

	Authenticate::Authenticate(const QString & ip,Uint16 port,
				const SHA1Hash & info_hash,const PeerID & peer_id) 
	: sock(0),info_hash(info_hash),our_peer_id(peer_id),done(false)
	{
		
		sock = new QSocket();
		connect(sock,SIGNAL(connected()),this,SLOT(connected()));
		connect(sock,SIGNAL(readyRead()),this,SLOT(readyRead()));
		connect(sock,SIGNAL(error(int)),this,SLOT(error(int )));
		
		host = ip;
		
		Out() << "Initiating connection to " << host << endl;
		sock->connectToHost(host,port);
		connect(&timer,SIGNAL(timeout()),this,SLOT(onTimeout()));
		timer.start(10000,true);
	}

	Authenticate::~Authenticate()
	{
		if (sock)
			delete sock;
	}
	
	void Authenticate::connected()
	{
		sendHandshake();
	}
	
	void Authenticate::error(int)
	{
		onFinish(false);
	}

	void Authenticate::onFinish(bool succes)
	{
		Out() << "Authentication to " << host << " : " << (succes ? "ok" : "failure") << endl;
		disconnect(sock,SIGNAL(connected()),this,SLOT(connected()));
		disconnect(sock,SIGNAL(readyRead()),this,SLOT(readyRead()));
		disconnect(sock,SIGNAL(error(int)),this,SLOT(error(int )));
		done = true;
		finished(this,succes);
		if (!succes)
		{
			delete sock;
			sock = 0;
		}
	}
	
	void Authenticate::readyRead()
	{
		if (done)
			return;
		
		if (sock->bytesAvailable() < 68)
			return;
		
		Uint8 hs[68];
		sock->readBlock((char*)hs,68);
		
		if (hs[0] != 19)
		{
			onFinish(false);
			return;
		}
		
		const char* pstr = "BitTorrent protocol";
		if (memcmp(pstr,hs+1,19) != 0)
		{
			onFinish(false);
			return;
		}
		
		SHA1Hash rh(hs+28);
		if (rh != info_hash)
		{
			onFinish(false);
			return;
		}
		
		char tmp[21];
		tmp[20] = '\0';
		memcpy(tmp,hs+48,20);
		peer_id = PeerID(tmp);
		
		if (our_peer_id == peer_id /*|| peer_id.startsWith("Yoda")*/)
		{
			onFinish(false);
			return;
		}
		
		onFinish(true);
	}

	QSocket* Authenticate::takeSocket()
	{
		QSocket* s = sock;
		sock = 0;
		return s;
	}
	
	void Authenticate::sendHandshake()
	{
		Uint8 hs[68];
		const char* pstr = "BitTorrent protocol";
		hs[0] = 19;
		memcpy(hs+1,pstr,19);
		memset(hs+20,0x00,8);
		memcpy(hs+28,info_hash.getData(),20);
		memcpy(hs+48,our_peer_id.data(),20);
		
		sock->writeBlock((const char*)hs,68);
	}
	
	void Authenticate::onTimeout()
	{
		Out() << "Timeout occured" << endl;
		onFinish(false);
	}
}
#include "authenticate.moc"
