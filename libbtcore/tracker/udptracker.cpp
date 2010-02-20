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
#include "udptracker.h"
#include <stdlib.h>
#include <arpa/inet.h>
#include <klocale.h>
#include <util/functions.h>
#include <util/log.h>
#include <k3socketaddress.h>
#include <k3resolver.h>
#include <peer/peermanager.h>
#include <interfaces/torrentinterface.h>
#include <torrent/globals.h>
#include <torrent/server.h>
#include "udptrackersocket.h"

using namespace KNetwork;

namespace bt
{
	
	UDPTrackerSocket* UDPTracker::socket = 0;
	Uint32 UDPTracker::num_instances = 0;
	

	UDPTracker::UDPTracker(const KUrl & url,TrackerDataSource* tds,const PeerID & id,int tier) 
	: Tracker(url,tds,id,tier)
	{
		num_instances++;
		if (!socket)
			socket = new UDPTrackerSocket();
		
		connection_id = 0;
		transaction_id = 0;
		interval = 0;
		scrape_transaction_id = 0;
		todo = NOTHING;
		
		conn_timer.setSingleShot(true);
		connect(&conn_timer,SIGNAL(timeout()),this,SLOT(onConnTimeout()));
		connect(socket,SIGNAL(announceReceived(Int32, const QByteArray & )),
				this,SLOT(announceReceived(Int32, const QByteArray & )));
		connect(socket,SIGNAL(connectReceived(Int32, Int64 )),
				this,SLOT(connectReceived(Int32, Int64 )));
		connect(socket,SIGNAL(error(Int32, const QString& )),
				this,SLOT(onError(Int32, const QString& )));
		connect(socket,SIGNAL(scrapeReceived(Int32,QByteArray)),
				this,SLOT(scrapeReceived(Int32,QByteArray)));
		
		resolved = false;
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
	
	void UDPTracker::start()
	{
		event = STARTED;
		resetTrackerStats();
		conn_timer.stop();
		doRequest();
	}
	
	void UDPTracker::stop(WaitJob* )
	{
		if (!started)
		{
			if (transaction_id) 
			{
				socket->cancelTransaction(transaction_id);
				transaction_id = 0;
				status = TRACKER_IDLE;
				requestOK();
			}
			reannounce_timer.stop();
		}
		else
		{
			event = STOPPED;
			reannounce_timer.stop();
			conn_timer.stop();
			doRequest();
			started = false;
		}
	}
	
	void UDPTracker::completed()
	{
		event = COMPLETED;
		conn_timer.stop();
		doRequest();
	}
	
	void UDPTracker::manualUpdate()
	{
		conn_timer.stop();
		if (!started)
			start();
		else
			doRequest();
	}

	void UDPTracker::connectReceived(Int32 tid,Int64 cid)
	{
		if (tid != transaction_id)
			return;
		
		connection_id = cid;
		failures = 0;
		if (todo & ANNOUNCE_REQUEST)
			sendAnnounce();
		if (todo & SCRAPE_REQUEST)
			sendScrape();
	}
	
	void UDPTracker::announceReceived(Int32 tid,const QByteArray & b)
	{
		if (tid != transaction_id || b.size() < 20)
			return;

		Uint8* buf = (Uint8*)b.data();
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
		for (int i = 20;i < b.size() && j < nip;i+=6,j++)
		{
			Uint32 ip = ReadUint32(buf,i);
			QString ip_str = QString("%1.%2.%3.%4")
				.arg((ip & 0xFF000000) >> 24)
				.arg((ip & 0x00FF0000) >> 16)
				.arg((ip & 0x0000FF00) >> 8)
				.arg(ip & 0x000000FF);
			addPeer(ip_str,ReadUint16(buf,i+4));
		}
		
		peersReady(this);
		connection_id = 0;
		conn_timer.stop();
		if (event != STOPPED)
		{
			if (event == STARTED)
				started = true;
			event = NONE;
			status = TRACKER_OK;
			requestOK();
			if (started)
				reannounce_timer.start(interval * 1000);
		}
		else
		{
			stopDone();
			status = TRACKER_IDLE;
			requestOK();
		}
		request_time = QDateTime::currentDateTime();
	}
	
	void UDPTracker::onError(Int32 tid,const QString & error_string)
	{
		if (tid != transaction_id)
			return;

		Out(SYS_TRK|LOG_IMPORTANT) << "UDPTracker::error : " << error_string << endl;
		failed(error_string);
	}


	bool UDPTracker::doRequest()
	{
		Out(SYS_TRK|LOG_NOTICE) << "Doing tracker request to url : " << url << endl;
		if (!resolved)
		{
			todo |= ANNOUNCE_REQUEST;
			KResolver::resolveAsync(this,SLOT(onResolverResults(KNetwork::KResolverResults )),
									url.host(),QString::number(url.port(80)));
		}
		else if (connection_id == 0)
		{
			todo |= ANNOUNCE_REQUEST;
			failures = 0;
			sendConnect();
		}
		else
		{
			sendAnnounce();
		}

		status = TRACKER_ANNOUNCING;
		requestPending();
		return true;
	}
	
	void UDPTracker::scrape()
	{
		Out(SYS_TRK|LOG_NOTICE) << "Doing scrape request to url : " << url << endl;
		if (!resolved)
		{
			todo |= SCRAPE_REQUEST;
			KResolver::resolveAsync(this,SLOT(onResolverResults(KNetwork::KResolverResults )),
									url.host(),QString::number(url.port(80)));
		}
		else if (connection_id == 0)
		{
			todo |= SCRAPE_REQUEST;
			failures = 0;
			sendConnect();
		}
		else
		{
			sendScrape();
		}
	}

	void UDPTracker::scrapeReceived(Int32 tid, const QByteArray& b)
	{
		/*
		0				32-bit integer	action	2
		4				32-bit integer	transaction_id
		8 + 12 * n		32-bit integer	seeders
		12 + 12 * n		32-bit integer	completed
		16 + 12 * n		32-bit integer	leechers
		8 + 12 * N 
		*/
		if (tid != scrape_transaction_id || b.size() < 20)
			return;
		
		Uint8* buf = (Uint8*)b.data();
		seeders = ReadInt32(buf,8);
		total_downloaded = ReadInt32(buf,12);
		leechers = ReadInt32(buf,16);
		Out(SYS_TRK|LOG_DEBUG) << "Scrape : leechers = " << leechers 
		<< ", seeders = " << seeders << ", downloaded = " << total_downloaded << endl;
	}

	void UDPTracker::sendConnect()
	{
		transaction_id = socket->newTransactionID();
		socket->sendConnect(transaction_id,address);
		int tn = 1;
		for (int i = 0;i < failures;i++)
			tn *= 2;
		conn_timer.start(60000 * tn);
	}

	void UDPTracker::sendAnnounce()
	{		
		todo &= ~ANNOUNCE_REQUEST;
	//	Out(SYS_TRK|LOG_NOTICE) << "UDPTracker::sendAnnounce()" << endl;
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

		Uint32 ev = event;
		Uint16 port = ServerInterface::getPort();
		Uint8 buf[98];
		WriteInt64(buf,0,connection_id);
		WriteInt32(buf,8,UDPTrackerSocket::ANNOUNCE);
		WriteInt32(buf,12,transaction_id);
		const SHA1Hash & info_hash = tds->infoHash();
		memcpy(buf+16,info_hash.getData(),20);
		memcpy(buf+36,peer_id.data(),20);
		WriteInt64(buf,56,bytesDownloaded());
		if (ev == COMPLETED)
			WriteInt64(buf,64,0);
		else
			WriteInt64(buf,64,tds->bytesLeft());
		WriteInt64(buf,72,bytesUploaded());
		WriteInt32(buf,80,ev);
		QString cip = Tracker::getCustomIP();
		if (cip.isNull())
		{
			WriteUint32(buf,84,0);
		}
		else
		{
			KNetwork::KIpAddress addr(cip);
			WriteUint32(buf,84,ntohl(addr.IPv4Addr(true)));
		}
		WriteUint32(buf,88,key);
		if (ev != STOPPED)
			WriteInt32(buf,92,100);
		else
			WriteInt32(buf,92,0);
		WriteUint16(buf,96,port);

		socket->sendAnnounce(transaction_id,buf,address);
	}
	
	
	void UDPTracker::sendScrape()
	{
		todo &= ~SCRAPE_REQUEST;
		/*
		0				64-bit integer	connection_id
		8				32-bit integer	action	2
		12				32-bit integer	transaction_id
		16 + 20 * n		20-byte string	info_hash
		16 + 20 * N 
		*/
		scrape_transaction_id = socket->newTransactionID();
		Uint8 buf[36];
		WriteInt64(buf,0,connection_id);
		WriteInt32(buf,8,UDPTrackerSocket::SCRAPE);
		WriteInt32(buf,12,scrape_transaction_id);
		const SHA1Hash & info_hash = tds->infoHash();
		memcpy(buf+16,info_hash.getData(),20);
		
		socket->sendScrape(scrape_transaction_id,buf,address);
	}


	void UDPTracker::onConnTimeout()
	{
		if (connection_id)
		{
			connection_id = 0;
			failures++;
			if (event != STOPPED)
				onError(transaction_id,i18n("Timeout contacting tracker %1",url.prettyUrl()));
			else
			{
				status = TRACKER_IDLE;
				stopDone();
			}
		}
		else
		{
			failures++;
			onError(transaction_id,i18n("Timeout contacting tracker %1",url.prettyUrl()));
		}
	}

	void UDPTracker::onResolverResults(KResolverResults res)
	{
		if (res.count() > 0)
		{
			address = res.front().address();
			resolved = true;
			// continue doing request
			if (connection_id == 0)
			{
				failures = 0;
				sendConnect();
			}
			else
			{
				if (todo & ANNOUNCE_REQUEST)
					sendAnnounce();
				if (todo & SCRAPE_REQUEST)
					sendScrape();
			}
		}
		else 
		{
			failures++;
			failed(i18n("Unable to resolve hostname %1",url.host()));
		}
	}
	
}
#include "udptracker.moc"
