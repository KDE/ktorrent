/***************************************************************************
 *   Copyright (C) 2005-2007 by Joris Guisson                              *
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
#ifndef KTUPNPMCASTSOCKET_H
#define KTUPNPMCASTSOCKET_H

#include <QSet>
#include <QUdpSocket>
#include <util/ptrmap.h>
#include <util/constants.h>
#include <ktupnp_export.h>
#include "upnprouter.h"

using bt::Uint32;

namespace kt
{
	class UPnPRouter;
	
	/**
	 * @author Joris Guisson
	 * 
	 * Socket used to discover UPnP devices. This class will keep track
	 * of all discovered devices. 
	*/
	class KTUPNP_EXPORT UPnPMCastSocket : public QUdpSocket
	{
	    Q_OBJECT
	public:
		UPnPMCastSocket(bool verbose = false);
		virtual ~UPnPMCastSocket();
		
		/// Get the number of routers discovered
		Uint32 getNumDevicesDiscovered() const {return routers.count();}
		
		/// Find a router using it's server name
		UPnPRouter* findDevice(const QString & name) {return routers.find(name);}
		
		/// Save all routers to a file (for convenience at startup)
		void saveRouters(const QString & file);
		
		/// Load all routers from a file
		void loadRouters(const QString & file);

		/// Set verbose mode
		void setVerbose(bool v) {verbose = v;}
		
	public slots:
		/**
		 * Try to discover a UPnP device on the network.
		 * A signal will be emitted when a device is found.
		 */
		void discover();
	
	private slots:
		void onReadyRead();
		void error(QAbstractSocket::SocketError err);
		void onXmlFileDownloaded(UPnPRouter* r,bool success);
		
	signals:
		/**
		 * Emitted when a router or internet gateway device is detected.
		 * @param router The router
		 */
		void discovered(kt::UPnPRouter* router);
		
	public:
		UPnPRouter* parseResponse(const QByteArray & arr);
		
	private:
		void joinUPnPMCastGroup();
		void leaveUPnPMCastGroup();
	
	private:	
		bt::PtrMap<QString,UPnPRouter> routers;
		QSet<UPnPRouter*> pending_routers; // routers which we are downloading the XML file from
		bool verbose;
	};
}

#endif
