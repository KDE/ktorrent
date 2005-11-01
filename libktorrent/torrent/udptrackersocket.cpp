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
#include <time.h>
#include <stdlib.h>
#include <util/array.h>
#include <qsocketdevice.h>
#include <qsocketnotifier.h>
#include <util/log.h>
#include <util/functions.h>
#include <klocale.h>
#include <kmessagebox.h>
#include "globals.h"
#include "udptrackersocket.h"

namespace bt
{
	Uint16 UDPTrackerSocket::port = 4444;

	UDPTrackerSocket::UDPTrackerSocket() 
	{
		sock = new QSocketDevice(QSocketDevice::Datagram);
		int i = 0;
		while (!sock->bind(QHostAddress("localhost"),port + i) && i < 10)
		{
			Out() << "Failed to bind socket to port " << (port+i) << endl;
			i++;
		}

		if (i > 0 && sock->isValid())
			KMessageBox::information(0,
				i18n("Specified udp port (%1) is unavailable or in"
					" use by another application. KTorrent is bound to port %2.")
					.arg(port).arg(port + i - 1));
		else if (i > 0 && !sock->isValid())
			KMessageBox::error(0,
				i18n("Cannot bind to udp port %1 or the 10 following ports.").arg(port));

		sn = new QSocketNotifier(sock->socket(),QSocketNotifier::Read);
		connect(sn,SIGNAL(activated(int)),this,SLOT(dataRecieved(int )));
	}
	
	
	UDPTrackerSocket::~UDPTrackerSocket()
	{
		delete sock;
		delete sn;
	}

	void UDPTrackerSocket::sendConnect(Int32 tid,const QHostAddress & addr,Uint16 udp_port)
	{
		Int64 cid = 0x41727101980LL;
		Uint8 buf[16];

		WriteInt64(buf,0,cid);
		WriteInt32(buf,8,CONNECT);
		WriteInt32(buf,12,tid);
		sock->writeBlock((const char*)buf,16,addr,udp_port);
		transactions.insert(tid,CONNECT);
	}

	void UDPTrackerSocket::sendAnnounce(Int32 tid,const Uint8* data,
										const QHostAddress & addr,Uint16 udp_port)
	{
		transactions.insert(tid,ANNOUNCE);
		sock->writeBlock((const char*)data,98,addr,udp_port);
	}

	void UDPTrackerSocket::cancelTransaction(Int32 tid)
	{
		transactions.remove(tid);
	}

	void UDPTrackerSocket::handleConnect(const Array<Uint8> & buf)
	{	
		// Read the transaction_id and check it
		Int32 tid = ReadInt32(buf,4);
		QMap<Int32,Action>::iterator i = transactions.find(tid);
		// if we can't find the transaction, just return
		if (i == transactions.end())
			return;

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

	void UDPTrackerSocket::handleAnnounce(const Array<Uint8> & buf)
	{
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
		announceRecieved(tid,buf);
	}
	
	void UDPTrackerSocket::handleError(const Array<Uint8> & buf)
	{
		// Read the transaction_id and check it
		Int32 tid = ReadInt32(buf,4);
		QMap<Int32,Action>::iterator it = transactions.find(tid);
		// if we can't find the transaction, just return
		if (it == transactions.end())
			return;

		// extract error message
		transactions.erase(it);
		QString msg;
		for (Uint32 i = 8;i < buf.size();i++)
			msg += (char)buf[i];

		// emit signal
		error(tid,msg);
	}

	void UDPTrackerSocket::dataRecieved(int)
	{
		Uint32 ba = sock->bytesAvailable();
		Array<Uint8> buf(ba);
		sock->readBlock((char*)(Uint8*)buf,ba);
		Uint32 type = ReadUint32(buf,0);
		switch (type)
		{
			case CONNECT:
				handleConnect(buf);
				break;
			case ANNOUNCE:
				handleAnnounce(buf);
				break;
			case ERROR:
				handleError(buf);
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
	
}

#include "udptrackersocket.moc"
