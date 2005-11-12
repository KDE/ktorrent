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
#ifndef KTUPNPMCASTSOCKET_H
#define KTUPNPMCASTSOCKET_H

#include <util/ptrmap.h>
#include <kdatagramsocket.h>
#include <util/constants.h>
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
	class UPnPMCastSocket : public KNetwork::KDatagramSocket
	{
	Q_OBJECT
	public:
		UPnPMCastSocket();
		virtual ~UPnPMCastSocket();
		
		/// Get the number of routers discovered
		Uint32 getNumDevicesDiscovered() const {return routers.count();}
		
		/// Find a router using it's server name
		UPnPRouter* findDevice(const QString & name) {return routers.find(name);}
		
	public slots:
		/**
		 * Try to discover a UPnP device on the network.
		 * A signal will be emitted when a device is found.
		 */
		void discover();
	
	private slots:
		void onReadyRead();
		
	signals:
		/**
		 * Emitted when a router or internet gateway device is detected.
		 * @param router The router
		 */
		void discovered(UPnPRouter* router);
		
	private:
		UPnPRouter* parseResponse(const QByteArray & arr);
	
	private:	
		bt::PtrMap<QString,UPnPRouter> routers;
	};
}

#endif
