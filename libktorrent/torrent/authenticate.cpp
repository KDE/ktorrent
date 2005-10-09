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
#include <util/log.h>
#include "authenticate.h"
#include "ipblocklist.h"

namespace bt
{

	Authenticate::Authenticate(const QString & ip,Uint16 port,
				const SHA1Hash & info_hash,const PeerID & peer_id) 
	: info_hash(info_hash),our_peer_id(peer_id)
	{
		finished = succes = false;
		sock = new QSocket();
		connect(sock,SIGNAL(connected()),this,SLOT(connected()));
		connect(sock,SIGNAL(readyRead()),this,SLOT(onReadyRead()));
		connect(sock,SIGNAL(error(int)),this,SLOT(onError(int )));
		
		host = ip;
		Out() << "Initiating connection to " << host << endl;
		sock->connectToHost(host,port);
	}

	Authenticate::~Authenticate()
	{
	}
	
	void Authenticate::connected()
	{
		sendHandshake(info_hash,our_peer_id);
	}

	void Authenticate::onFinish(bool succes)
	{
		Out() << "Authentication to " << host << " : " << (succes ? "ok" : "failure") << endl;
		disconnect(sock,SIGNAL(connected()),this,SLOT(connected()));
		disconnect(sock,SIGNAL(readyRead()),this,SLOT(onReadyRead()));
		disconnect(sock,SIGNAL(error(int)),this,SLOT(onError(int )));
		finished = true;
		this->succes = succes;
		if (!succes)
		{
			delete sock;
			sock = 0;
		}
		timer.stop();
	}
	
	void Authenticate::handshakeRecieved(const Uint8* hs)
	{
		IPBlocklist& ipfilter = IPBlocklist::instance();
			//Out() << "Dodo " << pp.ip << endl;
		if (ipfilter.isBlocked(host))
		{
			Out() << "IP-address " << host << " is blacklisted." << endl;
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
}
#include "authenticate.moc"
