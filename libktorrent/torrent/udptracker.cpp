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
#include <stdlib.h>
#include <util/functions.h>
#include <util/log.h>
#include "peermanager.h"
#include "udptracker.h"
#include "torrentcontrol.h"
#include "globals.h"
#include "server.h"
#include "udptrackersocket.h"

using namespace kt;

namespace bt
{
	
	UDPTrackerSocket* UDPTracker::socket = 0;
	Uint32 UDPTracker::num_instances = 0;
	

	UDPTracker::UDPTracker(kt::TorrentInterface* tor,const SHA1Hash & ih,const PeerID & pid) : Tracker(tor,ih,pid),n(0)
	{
		num_instances++;
		if (!socket)
			socket = new UDPTrackerSocket();
		
		connection_id = 0;
		transaction_id = 0;
		leechers = seeders = interval = 0;
		connect(&conn_timer,SIGNAL(timeout()),this,SLOT(onConnTimeout()));
		connect(socket,SIGNAL(announceRecieved(Int32, const Array< Uint8 >& )),
				this,SLOT(announceRecieved(Int32, const Array< Uint8 >& )));
		connect(socket,SIGNAL(connectRecieved(Int32, Int64 )),
				this,SLOT(connectRecieved(Int32, Int64 )));
		connect(socket,SIGNAL(error(Int32, const QString& )),
				this,SLOT(onError(Int32, const QString& )));
	}


	UDPTracker::~UDPTracker()
	{
		num_instances--;
		if (num_instances == 0)
		{
			delete socket;
			socket = 0;
		}
	}

	void UDPTracker::connectRecieved(Int32 tid,Int64 cid)
	{
		if (tid != transaction_id)
			return;
		
		connection_id = cid;
		n = 0;
		sendAnnounce();
	}
	
	void UDPTracker::announceRecieved(Int32 tid,const Array<Uint8> & buf)
	{
		if (tid != transaction_id)
			return;

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
		interval = ReadInt32(buf,8);
		leechers = ReadInt32(buf,12);
		seeders = ReadInt32(buf,16);

		Uint32 nip = leechers + seeders;
		Uint32 j = 0;
		for (Uint32 i = 20;i < buf.size(),j < nip;i+=6,j++)
		{
			PotentialPeer pp;
			pp.ip = QHostAddress(ReadUint32(buf,i)).toString();
			pp.port = ReadUint16(buf,i+4);
			ppeers.append(pp);
		}
		dataReady();
	}
	
	void UDPTracker::onError(Int32 tid,const QString & error_string)
	{
		if (tid != transaction_id)
			return;

		Out() << "UDPTracker::error : " << error_string << endl;
		error();
	}


	void UDPTracker::doRequest(const KURL & url)
	{
		if (old_url != url)
		{
			connection_id = 0;
		}

		Out() << "Doing tracker request to url : " << url << endl;
		addr = LookUpHost(url.host());
		udp_port = url.port();
		if (connection_id == 0)
		{
			n = 0;
			sendConnect();
		}
		else
			sendAnnounce();

		old_url = url;
	}

	void UDPTracker::sendConnect()
	{
		transaction_id = socket->newTransactionID();
		socket->sendConnect(transaction_id,addr,udp_port);
		int tn = 1;
		for (int i = 0;i < n;i++)
			tn *= 2;
		conn_timer.start(60000 * tn,true);
	}

	void UDPTracker::sendAnnounce()
	{
	//	Out() << "UDPTracker::sendAnnounce()" << endl;
		transaction_id = socket->newTransactionID();
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

		Uint32 ev = NONE;
		if (event == "started")
			ev = STARTED;
		else if (event == "completed")
			ev = COMPLETED;
		else if (event == "stopped")
			ev = STOPPED;

		const TorrentStats & s = tor->getStats();
		Uint16 port = Globals::instance().getServer().getPortInUse();
		Uint8 buf[98];
		WriteInt64(buf,0,connection_id);
		WriteInt32(buf,8,ANNOUNCE);
		WriteInt32(buf,12,transaction_id);
		memcpy(buf+16,info_hash.getData(),20);
		memcpy(buf+36,peer_id.data(),20);
		WriteInt64(buf,56,s.bytes_downloaded);
		WriteInt64(buf,64,s.bytes_left);
		WriteInt64(buf,72,s.bytes_uploaded);
		WriteInt32(buf,80,ev);
		WriteUint32(buf,84,0);
		WriteInt32(buf,88,0);// Wtf is the bloody key ?
		WriteInt32(buf,92,100);
		WriteUint16(buf,96,port);

		socket->sendAnnounce(transaction_id,buf,addr,udp_port);
	}

	void UDPTracker::onConnTimeout()
	{
		n++;
		sendConnect();
	}

	void UDPTracker::updateData(PeerManager* pman)
	{
		setInterval(interval);

		QValueList<PotentialPeer>::iterator i = ppeers.begin();
		while (i != ppeers.end())
		{
			pman->addPotentialPeer(*i);
			i++;
		}
		ppeers.clear();
		updateOK();
	}

	
}
#include "udptracker.moc"
