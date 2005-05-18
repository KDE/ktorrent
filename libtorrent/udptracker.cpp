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
#include <stdlib.h>
#include <qsocketnotifier.h>
#include <qsocketdevice.h>
#include "udptracker.h"
#include "log.h"
#include "torrentcontrol.h"
#include "globals.h"

namespace bt
{
	enum Event
	{
		NONE = 0,
		COMPLETED = 1,
		STARTED = 2,
		STOPPED = 3
	};

	enum Action
	{
		CONNECT = 0,
		ANNOUNCE = 1,
		SCRAPE = 2,
		ERROR = 3
	};

	UDPTracker::UDPTracker(TorrentControl* tc) : Tracker(tc)
	{
		sock = new QSocketDevice(QSocketDevice::Datagram);
		
		connection_id = 0;
		transaction_id = 0;
		if (!sock->bind(QHostAddress("localhost"),4444))
			Out() << "Failed to bind socket to port " << 4444 << endl;

		sn = new QSocketNotifier(sock->socket(),QSocketNotifier::Read);
		connect(sn,SIGNAL(activated(int)),this,SLOT(dataRecieved(int )));

		leechers = seeders = interval = 0;
		peer_buf = 0;
	}


	UDPTracker::~UDPTracker()
	{
		delete sock;
		delete sn;
		delete [] peer_buf;
	}


	void UDPTracker::doRequest(const KURL & url)
	{
		if (peer_buf)
			return;
		
		if (old_url != url)
		{
			connection_id = 0;
		}

		Out() << "DoRequest " << url.host() << ":" << url.port() << endl;
		addr = LookUpHost(url.host());
		Out() << addr.toString() << endl;
		udp_port = url.port();
		if (connection_id == 0)
			sendConnect();
		else
			sendAnnounce();

		old_url = url;
	}

	void UDPTracker::dataRecieved(int )
	{
		if (connection_id == 0)
		{
			connectRecieved();
		}
		else if (!peer_buf)
		{
			announceRecieved();
		}
		else
		{
			continueAnnounce();
		}
	}

	void UDPTracker::sendConnect()
	{
		Out() << "UDPTracker::sendConnect()" << endl;
		Uint64 cid = 0x41727101980ULL;
		transaction_id = rand() * time(0);
		Uint8 buf[16];

		WriteUint64(buf,0,cid);
		WriteUint32(buf,8,CONNECT);
		WriteUint32(buf,12,transaction_id);
		sock->writeBlock((const char*)buf,16,addr,udp_port);
	}

	
	void UDPTracker::connectRecieved()
	{
		Out() << "UDPTracker::connectRecieved()" << endl;
		/*
		0  32-bit integer  action  0
		4  32-bit integer  transaction_id
		8  64-bit integer  connection_id
		16 
		*/
		Uint8 buf[16];

		if (!sock->bytesAvailable() == 16)
		{
			handleError();
			return;
		}

		if (sock->readBlock((char*)buf,16) != 16)
		{
			handleError();
			return;
		}

		if (ReadUint32(buf,4) != transaction_id || ReadUint32(buf,0) != CONNECT)
		{
			handleError();
			return;
		}

		connection_id = ReadUint64(buf,8);
		sendAnnounce();
	}

	void UDPTracker::sendAnnounce()
	{
		Out() << "UDPTracker::sendAnnounce()" << endl;
		transaction_id = rand() * time(0);
		/*
		0  64-bit integer  connection_id
		8  32-bit integer  action  1
		12  32-bit integer  transaction_id
		16  20-byte string  info_hash
		36  20-byte string  peer_id
		56  64-bit integer  downloaded
		64  64-bit integer  left
		72  64-bit integer  uploaded
		80  32-bit integer  event
		84  32-bit integer  IP address  0
		88  32-bit integer  key
		92  32-bit integer  num_want  -1
		96  16-bit integer  port
		98
		*/
		Uint8 buf[98];
		WriteUint64(buf,0,connection_id);
		WriteUint32(buf,8,ANNOUNCE);
		WriteUint32(buf,12,transaction_id);
		memcpy(buf+16,info_hash.getData(),20);
		memcpy(buf+36,peer_id.data(),20);
		WriteUint64(buf,56,downloaded);
		WriteUint64(buf,64,left);
		WriteUint64(buf,72,uploaded);

		Uint32 ev = NONE;
		if (event == "started")
			ev = STARTED;
		else if (event == "completed")
			ev = COMPLETED;
		else if (event == "stopped")
			ev = STOPPED;
		
		WriteUint32(buf,80,ev);
		WriteUint32(buf,84,0);
		WriteUint32(buf,88,0);// Wtf is the bloody key ?
		WriteUint32(buf,92,-1);
		WriteUint16(buf,96,port);

		sock->writeBlock((const char*)buf,98,addr,udp_port);
	}

	void UDPTracker::announceRecieved()
	{
		Out() << "UDPTracker::announceRecieved()" << endl;
		/*
		0  32-bit integer  action  1
		4  32-bit integer  transaction_id
		8  32-bit integer  interval
		12  32-bit integer  leechers
		16  32-bit integer  seeders
		20 + 6 * n  32-bit integer  IP address
		24 + 6 * n  16-bit integer  TCP port
		20 + 6 * N
		*/
		Uint8 buf[20];

		if (sock->bytesAvailable() < 20)
		{
			handleError();
			return;
		}
		
		if (sock->readBlock((char*)buf,20) != 20)
		{
			handleError();
			return;
		}

		Uint32 action = ReadUint32(buf,0);
		if (ReadUint32(buf,4) != transaction_id || action != ANNOUNCE)
		{
			handleError();
			return;
		}

		interval = ReadUint32(buf,8);
		leechers = ReadUint32(buf,12);
		seeders = ReadUint32(buf,16);

		Out() << "bytesAvailable = " << sock->bytesAvailable() << endl;
		Out() << "seeders = " << seeders << endl;
		Out() << "leechers = " << leechers << endl;
		
		Uint32 nip = seeders + leechers;
		peer_buf = new Uint8[6*nip];
		data_read = 0;
		if ((Uint32)sock->bytesAvailable() >= 6*nip)
		{
			continueAnnounce();
		}
	}

	void UDPTracker::continueAnnounce()
	{
		Out() << "UDPTracker::continueAnnounce()" << endl;
		Uint32 nip = seeders + leechers;
		Uint32 ba = (Uint32)sock->bytesAvailable();
		Out() << ba << " " << (6*nip) << " " << sock->receiveBufferSize() << endl;
		if (data_read + ba< 6*nip)
		{
			sock->readBlock((char*)peer_buf+data_read,ba);
			data_read += ba;
		}
		else
		{
			sock->readBlock((char*)peer_buf+data_read,6*nip - data_read);
			tc->trackerResponse(interval,leechers,seeders,peer_buf);
			delete []peer_buf;
			peer_buf = 0;
			data_read = 0;
		}
	}

	void UDPTracker::handleError()
	{
		tc->trackerResponseError();
		Out() << "Error" << endl;
	}
}
#include "udptracker.moc"
