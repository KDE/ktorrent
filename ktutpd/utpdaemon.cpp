/***************************************************************************
 *   Copyright (C) 2009 by Joris Guisson                                   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#include <QDBusConnection>
#include "utpdaemon.h"
#include "utpdaemonadaptor.h"

namespace utp
{
	UTPDaemon::UTPDaemon(quint16 port,QObject* parent): QObject(parent),port(port),socket(0)
	{
		new UTPDaemonAdaptor(this);
		QDBusConnection dbus = QDBusConnection::sessionBus();
		dbus.registerObject("/UTPDaemon", this);
		dbus.registerService("org.ktorrent.UTPDaemon");
		socket = new QUdpSocket(this);
		connect(socket,SIGNAL(readyRead()),this,SLOT(handlePacket()));
	}
	
	UTPDaemon::~UTPDaemon()
	{
	}

	bool UTPDaemon::start()
	{
		if (!socket->bind(port,QUdpSocket::ShareAddress|QUdpSocket::ReuseAddressHint))
		{
			qWarning() << "Failed to bind to port " << port << ": " << socket->errorString();
			return false;
		}
		
		return true;
	}
		
	void UTPDaemon::handlePacket()
	{
		int ba = socket->bytesAvailable();
		QByteArray packet(ba,0);
		QHostAddress addr;
		quint16 packet_port = 0;
		if (socket->readDatagram(packet.data(),ba,&addr,&packet_port) == ba)
		{
		}
	}

	QString UTPDaemon::connectToPeer(const QString& ip, int port)
	{
		return QString();
	}

}
