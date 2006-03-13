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
#ifndef KTUPNPROUTER_H
#define KTUPNPROUTER_H

#include <kurl.h>
#include <qstringlist.h>
#include <kstreamsocket.h>
#include <util/constants.h>

using bt::Uint16;

namespace bt
{
	class HTTPRequest;
}

namespace kt 
{
	/**
	 * Structure describing a UPnP service found in an xml file.
	*/
	struct UPnPService	
	{ 
		QString serviceid;
		QString servicetype;
		QString controlurl;
		QString eventsuburl;
		QString scpdurl;
		
		UPnPService();
		UPnPService(const UPnPService & s);
		
		/**
		 * Set a property of the service.
		 * @param name Name of the property (matches to variable names)
		 * @param value Value of the property
		 */
		void setProperty(const QString & name,const QString & value);
		
		/**
		 * Set all strings to empty.
		 */
		void clear();
		
		/// Print the data of this service
		void debugPrintData();
		
		/**
		 * Assignment operator
		 * @param s The service to copy
		 * @return *this
		 */
		UPnPService & operator = (const UPnPService & s);
	};
	
	/**
	 *  Struct to hold the description of a device
	 */
	struct UPnPDeviceDescription
	{
		QString friendlyName;
		QString manufacturer;
		QString modelDescription;
		QString modelName;
		QString modelNumber;
		
		/**
		 * Set a property of the description
		 * @param name Name of the property (matches to variable names)
		 * @param value Value of the property
		 */
		void setProperty(const QString & name,const QString & value);
	};
	
	/**
	 * @author Joris Guisson
	 * 
	 * Class representing a UPnP enabled router. This class is also used to communicate
	 * with the router.
	*/
	class UPnPRouter : public QObject
	{
		Q_OBJECT
				
	public:
		enum Protocol
		{
			UDP,TCP
		};
		
		struct Forwarding
		{
			Uint16 port;
			Protocol prot;
			bool pending;
		};
	private:	
		QString server;
		KURL location;
		UPnPDeviceDescription desc;
		QValueList<UPnPService> services;
		QValueList<Forwarding> fwds;
		QMap<bt::HTTPRequest*,QValueList<Forwarding>::iterator > reqs;
	public:
		/**
		 * Construct a router.
		 * @param server The name of the router
		 * @param location The location of it's xml description file
		 */
		UPnPRouter(const QString & server,const KURL & location);	
		virtual ~UPnPRouter();
		
		/// Get the name  of the server
		QString getServer() const {return server;}
		
		/// Get the location of it's xml description
		KURL getLocation() const {return location;}
		
		/// Get the device description
		UPnPDeviceDescription & getDescription() {return desc;}
		
		/// Get the device description (const version)
		const UPnPDeviceDescription & getDescription() const {return desc;}
		
		/**
		 * Download the XML File of the router.
		 */
		bool downloadXMLFile();
		
		/**
		 * Add a service to the router.
		 * @param s The service
		 */
		void addService(const UPnPService & s);
			
		/**
		 * See if a port is forwarded
		 * @param port The Port
		 * @param protocol UDP or TCP
		 */
		void isPortForwarded(Uint16 port,Protocol protocol);	
		
		/**
		 * Get the external IP address.
		 */
		void getExternalIP();
		
		/**
		 * Forward a local port
		 * @param localip The local ip address
		 * @param port The local port to forward
		 * @param prot UDP or TCP
		 */
		void forward(Uint16 port,Protocol prot);
		
		/**
		 * Undo forwarding
		 * @param port The port
		 * @param prot UDP or TCP
		 */
		void undoForward(Uint16 remote,Protocol prot);
		
		void debugPrintData();
		
		QValueList<Forwarding>::iterator beginPortMappings() {return fwds.begin();}
		QValueList<Forwarding>::iterator endPortMappings() {return fwds.end();}
		
	private slots:
		void onReplyOK(bt::HTTPRequest* r,const QString &);
		void onReplyError(bt::HTTPRequest* r,const QString &);
		void onError(bt::HTTPRequest* r,bool);
		
		
	signals:
		/**
		 * Tell the GUI that it needs to be updated.
		 */
		void updateGUI();
		
	private:
		QValueList<UPnPService>::iterator findPortForwardingService();		
		
		bt::HTTPRequest* sendSoapQuery(const QString & query,const QString & soapact,const QString & controlurl);
	};

}

#endif
