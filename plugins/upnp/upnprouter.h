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
#ifndef KTUPNPROUTER_H
#define KTUPNPROUTER_H

#include <kurl.h>
#include <qstringlist.h>
#include <kstreamsocket.h>
#include <net/portlist.h>

using bt::Uint16;

namespace bt
{
	class HTTPRequest;
	class WaitJob;
}

namespace KIO
{
	class Job;
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
		struct Forwarding 
		{
			net::Port port;
			bt::HTTPRequest* pending_req;
			UPnPService* service;
		};
	private:	
		QString server;
		QString tmp_file;
		KURL location;
		UPnPDeviceDescription desc;
		QValueList<UPnPService> services;
		QValueList<Forwarding> fwds;
		QValueList<bt::HTTPRequest*> active_reqs;
	public:
		/**
		 * Construct a router.
		 * @param server The name of the router
		 * @param location The location of it's xml description file
		 * @param verbose Print lots of debug info
		 */
		UPnPRouter(const QString & server,const KURL & location,bool verbose = false);	
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
		void downloadXMLFile();
		
		/**
		 * Add a service to the router.
		 * @param s The service
		 */
		void addService(const UPnPService & s);
			
#if 0
		/**
		 * See if a port is forwarded
		 * @param port The Port
		 */
		void isPortForwarded(const net::Port & port);	
		
		/**
		 * Get the external IP address.
		 */
		void getExternalIP();
#endif
		
		/**
		 * Forward a local port
		 * @param port The local port to forward
		 */
		void forward(const net::Port & port);
		
		/**
		 * Undo forwarding
		 * @param port The port
		 * @param waitjob When this is set the jobs needs to be added to the waitjob, 
		 * so we can wait for their completeion at exit
		 */
		void undoForward(const net::Port & port,bt::WaitJob* waitjob = 0);
		
		void debugPrintData();
		
		QValueList<Forwarding>::iterator beginPortMappings() {return fwds.begin();}
		QValueList<Forwarding>::iterator endPortMappings() {return fwds.end();}
		
	private slots:
		void onReplyOK(bt::HTTPRequest* r,const QString &);
		void onReplyError(bt::HTTPRequest* r,const QString &);
		void onError(bt::HTTPRequest* r,bool);
		void downloadFinished(KIO::Job* j);
		
		
		
	signals:
		/**
		 * Tell the GUI that it needs to be updated.
		 */
		void updateGUI();
		
		/**
		 * Signal which indicates that the XML was downloaded successfully or not.
		 * @param r The router which emitted the signal
		 * @param success Wether or not it succeeded
		 */
		void xmlFileDownloaded(UPnPRouter* r,bool success);
		
	private:
		QValueList<UPnPService>::iterator findPortForwardingService();		
		
		bt::HTTPRequest* sendSoapQuery(const QString & query,const QString & soapact,const QString & controlurl,bool at_exit = false);
		bool verbose;
		
		void forward(UPnPService* srv,const net::Port & port);
		void undoForward(UPnPService* srv,const net::Port & port,bt::WaitJob* waitjob);
		void httpRequestDone(bt::HTTPRequest* r,bool erase_fwd);
	};

}

#endif
