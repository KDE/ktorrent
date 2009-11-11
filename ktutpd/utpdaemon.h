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

#ifndef UTP_UTPDAEMON_H
#define UTP_UTPDAEMON_H

#include <QObject>
#include <QUdpSocket>


namespace utp
{
	/**
		Class which implements the UTPDaemon dbus interface
	*/
	class UTPDaemon : public QObject
	{
		Q_OBJECT
		Q_CLASSINFO("D-Bus Interface", "org.ktorrent.UTPDaemon")
	public:
		UTPDaemon(quint16 port,QObject* parent = 0);
		virtual ~UTPDaemon();
		
		/// Start the daemon
		bool start();
		
	public Q_SLOTS:
		Q_SCRIPTABLE QString connectToPeer(const QString & ip,int port);
		
	Q_SIGNALS:
		Q_SCRIPTABLE void acceptedPeer(const QString & fifo,const QString & ip,int port);
		
	private Q_SLOTS:
		void handlePacket();
		
	private:
		quint16 port;
		QUdpSocket* socket;
	};

}

#endif // UTP_UTPDAEMON_H
