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
#include <klocale.h>
#include <qstringlist.h>
#include <kio/netaccess.h>
#include <torrent/globals.h>
#include <util/log.h>
#include <util/array.h>
#include <util/error.h>
#include <util/fileops.h>
#include "upnprouter.h"
#include "upnpdescriptionparser.h"
#include "soap.h"

using namespace bt;

namespace kt 
{
	UPnPService::UPnPService()
	{
	}
	
	UPnPService::UPnPService(const UPnPService & s)
	{
		this->servicetype = s.servicetype;
		this->controlurl = s.controlurl;
		this->eventsuburl = s.eventsuburl;
		this->serviceid = s.serviceid;
		this->scpdurl = s.scpdurl;
	}

	void UPnPService::setProperty(const QString & name,const QString & value)
	{
		if (name == "serviceType")
			servicetype = value;
		else if (name == "controlURL")
			controlurl = value;
		else if (name == "eventSubURL")
			eventsuburl = value;
		else if (name == "SCPDURL")
			scpdurl = value;
		else if (name == "serviceId")
			serviceid = value;
	}
	
	void UPnPService::clear()
	{
		servicetype = controlurl = eventsuburl = scpdurl = serviceid = "";
	}
	
	void UPnPService::debugPrintData()
	{
		Out() << "    servicetype = " << servicetype << endl;
		Out() << "    controlurl = " << controlurl << endl;
		Out() << "    eventsuburl = " << eventsuburl << endl;
		Out() << "    scpdurl = " << scpdurl << endl;
		Out() << "    serviceid = " << serviceid << endl;
	}
	
	UPnPService & UPnPService::operator = (const UPnPService & s)
	{
		this->servicetype = s.servicetype;
		this->controlurl = s.controlurl;
		this->eventsuburl = s.eventsuburl;
		this->serviceid = s.serviceid;
		this->scpdurl = s.scpdurl;
		return *this;
	}
	
	///////////////////////////////////////
	
	void UPnPDeviceDescription::setProperty(const QString & name,const QString & value)
	{
		if (name == "friendlyName")
			friendlyName = value;
		else if (name == "manufacturer")
			manufacturer = value;
		else if (name == "modelDescription")
			modelDescription = value;
		else if (name == "modelName")
			modelName = value;
		else if (name == "modelNumber")
			modelNumber == value;
	}
	
	///////////////////////////////////////
	
	UPnPRouter::UPnPRouter(const QString & server,const KURL & location) : server(server),location(location)
	{
		sock = 0;
		waiting_for_reply = false;
		createSocket();
	}
	
	
	UPnPRouter::~UPnPRouter()
	{
	}
	
	void UPnPRouter::addService(const UPnPService & s)
	{
		services.append(s);
	}
	
	bool UPnPRouter::downloadXMLFile()
	{
		QString target;
		// download the contents
		if (KIO::NetAccess::download(location,target,0))
		{
			// load in the file (target is always local)
			UPnPDescriptionParser desc_parse;
			bool ret = desc_parse.parse(target,this);
			if (!ret)
			{
				Out() << "Error parsing router description !" << endl;
			}
			// and remove the temp file
			KIO::NetAccess::removeTempFile(target);
			return ret;
		}
		else
		{
			return false;
		}
		return true;
	}
	
	void UPnPRouter::debugPrintData()
	{
		Out() << "UPnPRouter : " << endl;
		Out() << "Friendly name = " << desc.friendlyName << endl;
		Out() << "Manufacterer = " << desc.manufacturer << endl;
		Out() << "Model description = " << desc.modelDescription << endl;
		Out() << "Model name = " << desc.modelName << endl;
		Out() << "Model number = " << desc.modelNumber << endl;
		for (QValueList<UPnPService>::iterator i = services.begin();i != services.end();i++)
		{
			UPnPService & s = *i;
			Out() << "Service : " << endl;
			s.debugPrintData();
			Out() << "Done" << endl;
		}
		Out() << "Done" << endl;
	}
	
	QValueList<UPnPService>::iterator UPnPRouter::findPortForwardingService()
	{
		QValueList<UPnPService>::iterator i = services.begin();
		while (i != services.end())
		{
			UPnPService & s = *i;
			if (s.servicetype == "urn:schemas-upnp-org:service:WANIPConnection:1")
				return i;
			i++;
		}
		return services.end();
	}

	void UPnPRouter::getExternalIP()
	{
		// first find the right service
		QValueList<UPnPService>::iterator i = findPortForwardingService();
		if (i == services.end())
			throw Error(i18n("Cannot find port forwarding service, in the device's description !"));
		
		UPnPService & s = *i;
		QString action = "GetExternalIPAddress";
		QString comm = SOAP::createCommand(action,s.servicetype);
		sendSoapQuery(comm,s.servicetype + "#" + action,s.controlurl);
	}
	
	void UPnPRouter::forward(Uint16 port,Protocol prot)
	{
		// first find the right service
		QValueList<UPnPService>::iterator i = findPortForwardingService();
		if (i == services.end())
			throw Error(i18n("Cannot find port forwarding service, in the device's description !"));
		
		// add all the arguments for the command
		QValueList<SOAP::Arg> args;
		SOAP::Arg a;
		a.element = "NewRemoteHost";
		args.append(a);
		
		// the external port
		a.element = "NewExternalPort";
		a.value = QString::number(port);
		args.append(a);
		
		// the protocol
		a.element = "NewProtocol";
		a.value = prot == TCP ? "TCP" : "UDP";
		args.append(a);
		
		// the local port
		a.element = "NewInternalPort";
		a.value = QString::number(port);
		args.append(a);
		
		// the local IP address
		a.element = "NewInternalClient";
		a.value = sock->localAddress().nodeName();
		args.append(a);
		
		a.element = "NewEnabled";
		a.value = "1";
		args.append(a);
		
		a.element = "NewPortMappingDescription";
		a.value = "KTorrent UPNP";	// TODO: change this
		args.append(a);
		
		a.element = "NewLeaseDuration";
		a.value = "0";
		args.append(a);
		
		UPnPService & s = *i;
		QString action = "AddPortMapping";
		QString comm = SOAP::createCommand(action,s.servicetype,args);
		Forwarding fw = {port,prot,true};
		fwds.append(fw);
		sendSoapQuery(comm,s.servicetype + "#" + action,s.controlurl);
	}
	
	void UPnPRouter::undoForward(Uint16 port,Protocol prot)
	{
		// first find the right service
		QValueList<UPnPService>::iterator i = findPortForwardingService();
		if (i == services.end())
			throw Error(i18n("Cannot find port forwarding service, in the device's description !"));
		
		// add all the arguments for the command
		QValueList<SOAP::Arg> args;
		SOAP::Arg a;
		a.element = "NewRemoteHost";
		args.append(a);
		
		// the external port
		a.element = "NewExternalPort";
		a.value = QString::number(port);
		args.append(a);
		
		// the protocol
		a.element = "NewProtocol";
		a.value = prot == TCP ? "TCP" : "UDP";
		args.append(a);
		
		UPnPService & s = *i;
		QString action = "DeletePortMapping";
		QString comm = SOAP::createCommand(action,s.servicetype,args);
		sendSoapQuery(comm,s.servicetype + "#" + action,s.controlurl);
	}
	
	void UPnPRouter::isPortForwarded(Uint16 port,Protocol prot)
	{
		// first find the right service
		QValueList<UPnPService>::iterator i = findPortForwardingService();
		if (i == services.end())
			throw Error(i18n("Cannot find port forwarding service, in the device's description !"));
		
		// add all the arguments for the command
		QValueList<SOAP::Arg> args;
		SOAP::Arg a;
		a.element = "NewRemoteHost";
		args.append(a);
		
		// the external port
		a.element = "NewExternalPort";
		a.value = QString::number(port);
		args.append(a);
		
		// the protocol
		a.element = "NewProtocol";
		a.value = prot == TCP ? "TCP" : "UDP";
		args.append(a);
		
		UPnPService & s = *i;
		QString action = "GetSpecificPortMappingEntry";
		QString comm = SOAP::createCommand(action,s.servicetype,args);
		sendSoapQuery(comm,s.servicetype + "#" + action,s.controlurl);
	}
	
	void UPnPRouter::createSocket()
	{
		if (sock)
			return;
		
		sock = new KNetwork::KStreamSocket(location.host(),QString::number(location.port()),this,0);
		sock->enableRead(true);
		sock->enableWrite(true);
		sock->setTimeout(30000);
		sock->setBlocking(false);
		connect(sock,SIGNAL(readyRead()),this,SLOT(onReadyRead()));
		connect(sock,SIGNAL(gotError(int)),this,SLOT(onSocketError(int )));
		connect(sock,SIGNAL(timedOut()),this,SLOT(onSocketTimeout()));
	
		if (!sock->connect())
		{
			QString err = i18n("Cannot create network connection to %1 : %2")
					.arg(location.host()).arg(sock->errorString());
			delete sock;
			sock = 0;
			throw Error(err);
		}
	}
	
	void UPnPRouter::sendSoapQuery(const QString & query,const QString & soapact,const QString & controlurl)
	{
		if (!sock)
			createSocket();
		
		QString http_hdr = QString(
				"POST %1 HTTP/1.1\r\n"
				"HOST: %2:%3\r\n"
				"Content-length: %4\r\n"
				"Content-Type: text/xml\r\n"
				"SOAPAction: \"%5\"\r\n"
				"\r\n").arg(controlurl).arg(location.host()).arg(location.port()).arg(query.length()).arg(soapact);

		
		QString msg = http_hdr + query;
		if (query_buf.empty())
		{
			// send the query
			sock->writeBlock(msg.ascii(),msg.length());
			Out() << "Sending SOAP query : " << endl;
			Out() << http_hdr << endl << query << endl << "Done" << endl;
			waiting_for_reply = true;
		}
		else
		{
			Out() << "Appending msg" << endl;
			query_buf.append(msg);
		}
	}
	
	void UPnPRouter::onReadyRead()
	{
		Uint32 ba = sock->bytesAvailable();
		if (ba == 0)
			return;
		Array<char> data(ba);
		ba = sock->readBlock(data,ba);
		QString strdata((const char*)data);
		QStringList sl = QStringList::split("\r\n",strdata,false);	
		
		
		if (sl.first().contains("HTTP") && sl.first().contains("200"))
		{
			// if data contains AddPortMapping
			// we now that a pending portforwarding is ok
			if (strdata.contains("AddPortMapping"))
			{
				QValueList<Forwarding>::iterator i = fwds.begin();
				while (i != fwds.end())
				{
					Forwarding & fw = *i;
					if (fw.pending)
					{
						fw.pending = false;
						break;
					}
					i++;
				}
			}
			replyOK(sl.last());
		}
		else
		{
			// remove the first pending port mapping
			if (strdata.contains("AddPortMapping"))
			{
				QValueList<Forwarding>::iterator i = fwds.begin();
				while (i != fwds.end())
				{
					Forwarding & fw = *i;
					if (fw.pending)
					{
						fwds.erase(i);
						break;
					}
					i++;
				}
			}
			replyError(sl.last());
		}
		waiting_for_reply = false;
		
		
		
		if (!query_buf.empty())
		{
			QString msg = query_buf.front();
			query_buf.erase(query_buf.begin());
			sock->writeBlock(msg.ascii(),msg.length());
			waiting_for_reply = true;
		}
	}
	
	void UPnPRouter::onSocketError(int)
	{
		Out() << "Error : " << sock->errorString() << endl;
		sock->deleteLater();
		sock = 0;
	}
	
	void UPnPRouter::onSocketTimeout()
	{
		Out() << "UPnP Timeout !" << endl;
		waiting_for_reply = false;
	}
	
}

#include "upnprouter.moc"
