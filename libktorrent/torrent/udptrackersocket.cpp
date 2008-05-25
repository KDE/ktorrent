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
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <util/array.h>
#include <ksocketaddress.h>
#include <kdatagramsocket.h>
#include <ksocketdevice.h>
#include <net/portlist.h>
#include <util/log.h>
#include <util/functions.h>
#include <klocale.h>
#include <kmessagebox.h>
#include "globals.h"
#include "udptrackersocket.h"
		
using namespace KNetwork;

namespace bt
{
	Uint16 UDPTrackerSocket::port = 4444;

	UDPTrackerSocket::UDPTrackerSocket() 
	{
		sock = new KNetwork::KDatagramSocket(this);
		sock->setAddressReuseable(true);
		connect(sock,SIGNAL(readyRead()),this,SLOT(dataReceived()));
		int i = 0;
		if (port == 0)
			port = 4444;
		
		bool bound = false;
		
		while (!(bound = sock->bind(QString::null,QString::number(port + i))) && i < 10)
		{
			Out() << "Failed to bind socket to port " << (port+i) << endl;
			i++;
		}
		

		if (!bound)
		{
			KMessageBox::error(0,
				i18n("Cannot bind to udp port %1 or the 10 following ports.").arg(port));
		}
		else
		{
			port = port + i;
			Globals::instance().getPortList().addNewPort(port,net::UDP,true);
		}
	}
	
	
	UDPTrackerSocket::~UDPTrackerSocket()
	{
		Globals::instance().getPortList().removePort(port,net::UDP);
		delete sock;
	}

	void UDPTrackerSocket::sendConnect(Int32 tid,const KNetwork::KSocketAddress & addr)
	{
		Int64 cid = 0x41727101980LL;
		Uint8 buf[16];

		WriteInt64(buf,0,cid);
		WriteInt32(buf,8,CONNECT);
		WriteInt32(buf,12,tid);
		
		sock->send(KDatagramPacket((char*)buf,16,addr));
		transactions.insert(tid,CONNECT);
	}

	void UDPTrackerSocket::sendAnnounce(Int32 tid,const Uint8* data,const KNetwork::KSocketAddress & addr)
	{
		transactions.insert(tid,ANNOUNCE);
		sock->send(KDatagramPacket((char*)data,98,addr));
	}

	void UDPTrackerSocket::cancelTransaction(Int32 tid)
	{
		transactions.remove(tid);
	}

	void UDPTrackerSocket::handleConnect(const QByteArray & data)
	{	
		const Uint8* buf = (const Uint8*)data.data();
		
		// Read the transaction_id and check it
		Int32 tid = ReadInt32(buf,4);
		QMap<Int32,Action>::iterator i = transactions.find(tid);
		// if we can't find the transaction, just return
		if (i == transactions.end())
		{
			return;
		}

		// check wether the transaction is a CONNECT
		if (i.data() != CONNECT)
		{
			transactions.erase(i);
			error(tid,QString::null);
			return;
		}

		// everything ok, emit signal
		transactions.erase(i);
		connectRecieved(tid,ReadInt64(buf,8));
	}

	void UDPTrackerSocket::handleAnnounce(const QByteArray & data)
	{
		const Uint8* buf = (const Uint8*)data.data();
		
		// Read the transaction_id and check it
		Int32 tid = ReadInt32(buf,4);
		QMap<Int32,Action>::iterator i = transactions.find(tid);
		// if we can't find the transaction, just return
		if (i == transactions.end())
			return;

		// check wether the transaction is a ANNOUNCE
		if (i.data() != ANNOUNCE)
		{
			transactions.erase(i);
			error(tid,QString::null);
			return;
		}

		// everything ok, emit signal
		transactions.erase(i);
		announceRecieved(tid,data);
	}
	
	void UDPTrackerSocket::handleError(const QByteArray & data)
	{
		const Uint8* buf = (const Uint8*)data.data();
		// Read the transaction_id and check it
		Int32 tid = ReadInt32(buf,4);
		QMap<Int32,Action>::iterator it = transactions.find(tid);
		// if we can't find the transaction, just return
		if (it == transactions.end())
			return;

		// extract error message
		transactions.erase(it);
		QString msg;
		for (Uint32 i = 8;i < data.size();i++)
			msg += (char)buf[i];

		// emit signal
		error(tid,msg);
	}

	void UDPTrackerSocket::dataReceived()
	{
		if (sock->bytesAvailable() == 0)
		{
			Out(SYS_TRK|LOG_NOTICE) << "0 byte UDP packet " << endl;
			// KDatagramSocket wrongly handles UDP packets with no payload
			// so we need to deal with it oursleves
			int fd = sock->socketDevice()->socket();
			char tmp;
			read(fd,&tmp,1);
			return;
		}
		
		KDatagramPacket pck = sock->receive();
		const QByteArray & data = pck.data();
		const Uint8* buf = (const Uint8*)data.data();
		Uint32 type = ReadUint32(buf,0);
		switch (type)
		{
			case CONNECT:
				handleConnect(data);
				break;
			case ANNOUNCE:
				handleAnnounce(data);
				break;
			case ERROR:
				handleError(data);
				break;
		}
	}

	Int32 UDPTrackerSocket::newTransactionID()
	{
		Int32 transaction_id = rand() * time(0);
		while (transactions.contains(transaction_id))
			transaction_id++;
		return transaction_id;
	}

	void UDPTrackerSocket::setPort(Uint16 p)
	{
		port = p;
	}
	
	Uint16 UDPTrackerSocket::getPort()
	{
		return port;
	}
}

#include "udptrackersocket.moc"
