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
#include <qfile.h>
#include <plugins/upnp/upnprouter.h>
#include <plugins/upnp/upnpdescriptionparser.h>
#include <util/fileops.h>
#include <util/error.h>
#include <util/log.h>
#include <torrent/globals.h>
#include "upnpparsedescriptiontest.h"

using namespace kt;
using namespace bt;

namespace utest
{
	static char* test_data1 = "<?xml version=\"1.0\"?>\n"
			"<root xmlns=\"urn:schemas-upnp-org:device-1-0\">\n"
			"<specVersion>\n"
			"<major>1</major>\n"
			"<minor>0</minor>\n"
			"</specVersion>\n"
			"<URLBase>http://192.168.0.1:5678</URLBase>\n"
			"<device>\n"
			"<deviceType>urn:schemas-upnp-org:device:InternetGatewayDevice:1</deviceType>\n"
			"<presentationURL>http://192.168.0.1:80</presentationURL>\n"
			"<friendlyName>D-Link Router</friendlyName>\n"
			"<manufacturer>D-Link</manufacturer>\n"
			"<manufacturerURL>http://www.dlink.com</manufacturerURL>\n"
			"<modelDescription>Internet Access Router</modelDescription>\n"
			"<modelName>D-Link Router</modelName>\n"
			"<UDN>uuid:upnp-InternetGatewayDevice-1_0-12345678900001</UDN>\n"
			"<UPC>123456789001</UPC>\n"
			"<serviceList>\n"
			"<service>\n"
			"<serviceType>urn:schemas-upnp-org:service:Layer3Forwarding:1</serviceType>\n"
			"<serviceId>urn:upnp-org:serviceId:L3Forwarding1</serviceId>\n"
			"<controlURL>/Layer3Forwarding</controlURL>\n"
			"<eventSubURL>/Layer3Forwarding</eventSubURL>\n"
			"<SCPDURL>/Layer3Forwarding.xml</SCPDURL>\n"
			"</service>\n"
			"</serviceList>\n"
			"<deviceList>\n"
			"<device>\n"
			"<deviceType>urn:schemas-upnp-org:device:WANDevice:1</deviceType>\n"
			"<friendlyName>WANDevice</friendlyName>\n"
			"<manufacturer>D-Link</manufacturer>\n"
			"<manufacturerURL>http://www.dlink.com</manufacturerURL>\n"
			"<modelDescription>Internet Access Router</modelDescription>\n"
			"<modelName>D-Link Router</modelName>\n"
			"<modelNumber>1</modelNumber>\n"
			"<modelURL>http://support.dlink.com</modelURL>\n"
			"<serialNumber>12345678900001</serialNumber>\n"
			"<UDN>uuid:upnp-WANDevice-1_0-12345678900001</UDN>\n"
			"<UPC>123456789001</UPC>\n"
			"<serviceList>\n"
			"<service>\n"
			"<serviceType>urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1</serviceType>\n"
			"<serviceId>urn:upnp-org:serviceId:WANCommonInterfaceConfig</serviceId>\n"
			"<controlURL>/WANCommonInterfaceConfig</controlURL>\n"
			"<eventSubURL>/WANCommonInterfaceConfig</eventSubURL>\n"
			"<SCPDURL>/WANCommonInterfaceConfig.xml</SCPDURL>\n"
			"</service>\n"
			"</serviceList>\n"
			"<deviceList>\n"
			"<device>\n"
			"<deviceType>urn:schemas-upnp-org:device:WANConnectionDevice:1</deviceType>\n"
			"<friendlyName>WAN Connection Device</friendlyName>\n"
			"<manufacturer>D-Link</manufacturer>\n"
			"<manufacturerURL>http://www.dlink.com</manufacturerURL>\n"
			"<modelDescription>Internet Access Router</modelDescription>\n"
			"<modelName>D-Link Router</modelName> \n"
			"<modelNumber>1</modelNumber>\n"
			"<modelURL>http://support.dlink.com</modelURL>\n"
			"<serialNumber>12345678900001</serialNumber>\n"
			"<UDN>uuid:upnp-WANConnectionDevice-1_0-12345678900001</UDN>\n"
			"<UPC>123456789001</UPC>\n"
			"<serviceList>\n"
			"<service>\n"
			"<serviceType>urn:schemas-upnp-org:service:WANIPConnection:1</serviceType>\n"
			"<serviceId>urn:upnp-org:serviceId:WANIPConnection</serviceId> \n"
			"<controlURL>/WANIPConnection</controlURL>\n"
			"<eventSubURL>/WANIPConnection</eventSubURL>\n"
			"<SCPDURL>/WANIPConnection.xml</SCPDURL>\n"
			"</service>\n"
			"</serviceList>\n"
			"</device>\n"
			"</deviceList>\n"
			"</device>\n"
			"</deviceList>\n"
			"</device>\n"
			"</root>";
	
	static const char* test_data2 = "<?xml version=\"1.0\"?> \n"
			"<root xmlns=\"urn:schemas-upnp-org:device-1-0\"> \n"
			"<specVersion> \n"
			"<major>1</major> \n"
			"<minor>0</minor> \n"
			"</specVersion> \n"
			"<URLBase>http://192.168.1.1:52869</URLBase> \n"
			"<device> \n"
			"<deviceType>urn:schemas-upnp-org:device:InternetGatewayDevice:1</deviceType> \n"
			"<friendlyName>DLINK Internet Gateway Device</friendlyName> \n"
			"<manufacturer>DLINK</manufacturer> \n"
			"<manufacturerURL>http://www.dlink.com</manufacturerURL> \n"
			"<modelName>DLINK IGD</modelName> \n"
			"<UDN>uuid:75802409-bccb-40e7-8e6c-fa095ecce13e</UDN> \n"
			"<iconList> \n"
			"<icon> \n"
			"<mimetype>image/gif</mimetype> \n"
			"<width>118</width> \n"
			"<height>119</height> \n"
			"<depth>8</depth> \n"
			"<url>/ligd.gif</url> \n"
			"</icon> \n"
			"</iconList> \n"
			"<serviceList> \n"
			"<service> \n"
			"<serviceType>urn:schemas-microsoft-com:service:OSInfo:1</serviceType> \n"
			"<serviceId>urn:microsoft-com:serviceId:OSInfo1</serviceId> \n"
			"<controlURL>/upnp/control/OSInfo1</controlURL> \n"
			"<eventSubURL>/upnp/event/OSInfo1</eventSubURL> \n"
			"<SCPDURL>/gateinfoSCPD.xml</SCPDURL> \n"
			"</service> \n"
			"</serviceList> \n"
			"<deviceList> \n"
			"<device> \n"
			"<deviceType>urn:schemas-upnp-org:device:WANDevice:1</deviceType> \n"
			"<friendlyName>WANDevice</friendlyName> \n"
			"<manufacturer>DLINK</manufacturer> \n"
			"<manufacturerURL>http://www.dlink.com</manufacturerURL> \n"
			"<modelDescription>WAN Device on DLINK IGD</modelDescription> \n"
			"<modelName>DLINK IGD</modelName> \n"
			"<modelNumber>0.92</modelNumber> \n"
			"<modelURL>http://www.dlink.com</modelURL> \n"
			"<serialNumber>0.92</serialNumber> \n"
			"<UDN>uuid:75802409-bccb-40e7-8e6c-fa095ecce13e</UDN> \n"
			"<UPC>DLINK IGD</UPC> \n"
			"<serviceList> \n"
			"<service> \n"
			"<serviceType>urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1</serviceType> \n"
			"<serviceId>urn:upnp-org:serviceId:WANCommonIFC1</serviceId> \n"
			"<controlURL>/upnp/control/WANCommonIFC1</controlURL> \n"
			"<eventSubURL>/upnp/control/WANCommonIFC1</eventSubURL> \n"
			"<SCPDURL>/gateicfgSCPD.xml</SCPDURL> \n"
			"</service> \n"
			"</serviceList> \n"
			"<deviceList> \n"
			"<device> \n"
			"<deviceType>urn:schemas-upnp-org:device:WANConnectionDevice:1</deviceType> \n"
			"<friendlyName>WANConnectionDevice</friendlyName> \n"
			"<manufacturer>DLINK</manufacturer> \n"
			"<manufacturerURL>http://www.dlink.com</manufacturerURL> \n"
			"<modelDescription>WanConnectionDevice on DLINK IGD</modelDescription> \n"
			"<modelName>DLINK IGD</modelName> \n"
			"<modelNumber>0.92</modelNumber> \n"
			"<modelURL>http://www.dlink.com</modelURL> \n"
			"<serialNumber>0.92</serialNumber> \n"
			"<UDN>uuid:75802409-bccb-40e7-8e6c-fa095ecce13e</UDN> \n"
			"<UPC>DLINK IGD</UPC> \n"
			"<serviceList> \n"
			"<service> \n"
			"<serviceType>urn:schemas-upnp-org:service:WANIPConnection:1</serviceType> \n"
			"<serviceId>urn:upnp-org:serviceId:WANIPConn1</serviceId> \n"
			"<controlURL>/upnp/control/WANIPConn1</controlURL> \n"
			"<eventSubURL>/upnp/control/WANIPConn1</eventSubURL> \n"
			"<SCPDURL>/gateconnSCPD.xml</SCPDURL> \n"
			"</service> \n"
			"</serviceList> \n"
			"</device> \n"
			"</deviceList> \n"
			"</device> \n"
			"</deviceList> \n"
			"<presentationURL>http://192.168.1.1/</presentationURL> \n"
			"</device> \n"
			"</root> ";
	
	static const char* test_data3 = "<?xml version=\"1.0\"?> \
			<root xmlns=\"urn:schemas-upnp-org:device-1-0\">  \
			<specVersion> \
			<major>1</major> \
			<minor>0</minor> \
			</specVersion> \
			<URLBase>http://192.168.0.5:5431/</URLBase> \
			<device> \
			<deviceType>urn:schemas-upnp-org:device:InternetGatewayDevice:1</deviceType> \
			<presentationURL>http://192.168.0.5:80/</presentationURL> \
			<friendlyName>Dynalink Wireless ADSL Router</friendlyName> \
			<manufacturer>Danalink</manufacturer> \
			<manufacturerURL>http://www.dynalink.co.nz/</manufacturerURL> \
			<modelDescription>Broadcom single-chip ADSL router</modelDescription> \
			<modelName>BCM6345+BCM4306</modelName> \
			<modelNumber>1.0</modelNumber> \
			<modelURL>http://www.dynalink.co.nz/</modelURL> \
			<UDN>uuid:10740000-0000-1000-b710-107c0032dca6</UDN> \
			<serviceList> \
			<service> \
			<serviceType>urn:schemas-upnp-org:service:Layer3Forwarding:1</serviceType> \
			<serviceId>urn:upnp-org:serviceId:Layer3Forwarding:11</serviceId> \
			<controlURL>/uuid:10740000-0000-1000-b710-107c0032dca6/Layer3Forwarding:1</controlURL> \
			<eventSubURL>/uuid:10740000-0000-1000-b710-107c0032dca6/Layer3Forwarding:1</eventSubURL> \
			<SCPDURL>/dynsvc/Layer3Forwarding:1.xml</SCPDURL> \
			</service> \
			</serviceList> \
			<deviceList> \
			<device> \
			<deviceType>urn:schemas-upnp-org:device:WANDevice:1</deviceType> \
			<friendlyName>urn:schemas-upnp-org:device:WANDevice:1</friendlyName> \
			<manufacturer>Danalink</manufacturer> \
			<manufacturerURL>http://www.dynalink.co.nz/</manufacturerURL> \
			<modelDescription>Broadcom single-chip ADSL router</modelDescription> \
			<modelName>BCM6345+BCM4306</modelName> \
			<modelNumber>1.0</modelNumber> \
			<modelURL>http://www.dynalink.co.nz/</modelURL> \
			<UDN>uuid:10740000-0000-1000-b710-107c0132dca6</UDN> \
			<serviceList> \
			<service> \
			<serviceType>urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1</serviceType> \
			<serviceId>urn:upnp-org:serviceId:WANCommonIFC1</serviceId> \
			<controlURL>/uuid:10740000-0000-1000-b710-107c0132dca6/WANCommonInterfaceConfig:1</controlURL> \
			<eventSubURL>/uuid:10740000-0000-1000-b710-107c0132dca6/WANCommonInterfaceConfig:1</eventSubURL> \
			<SCPDURL>/dynsvc/WANCommonInterfaceConfig:1.xml</SCPDURL> \
			</service> \
			</serviceList> \
			<deviceList> \
			<device> \
			<deviceType>urn:schemas-upnp-org:device:WANConnectionDevice:1</deviceType> \
			<friendlyName>urn:schemas-upnp-org:device:WANConnectionDevice:1</friendlyName> \
			<manufacturer>Danalink</manufacturer> \
			<manufacturerURL>http://www.dynalink.co.nz/</manufacturerURL> \
			<modelDescription>Broadcom single-chip ADSL router</modelDescription> \
			<modelName>BCM6345+BCM4306</modelName> \
			<modelNumber>1.0</modelNumber> \
			<modelURL>http://www.dynalink.co.nz/</modelURL> \
			<UDN>uuid:10740000-0000-1000-b710-107c0232dca6</UDN> \
			<serviceList> \
			<service> \
			<serviceType>urn:schemas-upnp-org:service:WANPPPConnection:1</serviceType> \
			<serviceId>urn:upnp-org:serviceId:WANPPPConn1</serviceId> \
			<controlURL>/uuid:10740000-0000-1000-b710-107c0232dca6/WANPPPConnection:1</controlURL> \
			<eventSubURL>/uuid:10740000-0000-1000-b710-107c0232dca6/WANPPPConnection:1</eventSubURL> \
			<SCPDURL>/dynsvc/WANPPPConnection:1.xml</SCPDURL> \
			</service> \
			</serviceList> \
			</device> \
			</deviceList> \
			</device> \
			</deviceList> \
			</device> \
			</root> ";
	
	const char* test_data4 = "<?xml version=\"1.0\"?>  \
			<root xmlns=\"urn:schemas-upnp-org:device-1-0\"> \
			<specVersion> \
			<major>1</major> \
			<minor>0</minor> \
			</specVersion> \
			<URLBase>http://192.168.1.1:2869</URLBase> \
			<device> \
			<deviceType>urn:schemas-upnp-org:device:InternetGatewayDevice:1</deviceType> \
			<friendlyName>OpenWrt Linux Internet Gateway Device</friendlyName> \
			<manufacturer>OpenWrt Project</manufacturer> \
			<manufacturerURL>http://www.openwrt.org</manufacturerURL> \
			<modelName>WRT54G(S)</modelName> \
			<UDN>uuid:75802409-bccb-40e7-8e6c-fa095ecce13e</UDN> \
			<iconList> \
			<icon> \
			<mimetype>image/gif</mimetype> \
			<width>118</width> \
			<height>119</height>\
			<depth>8</depth> \
			<url>/ligd.gif</url> \
			</icon> \
			</iconList> \
			<serviceList> \
			<service> \
			<serviceType>urn:schemas-microsoft-com:service:OSInfo:1</serviceType> \
			<serviceId>urn:microsoft-com:serviceId:OSInfo1</serviceId> \
			<controlURL>/upnp/control/OSInfo1</controlURL> \
			<eventSubURL>/upnp/event/OSInfo1</eventSubURL> \
			<SCPDURL>/gateinfoSCPD.xml</SCPDURL> \
			</service> \
			</serviceList> \
			<deviceList> \
			<device> \
			<deviceType>urn:schemas-upnp-org:device:WANDevice:1</deviceType> \
			<friendlyName>WANDevice</friendlyName> \
			<manufacturer>OpenWrt Project</manufacturer> \
			<manufacturerURL>http://www.openwrt.org</manufacturerURL> \
			<modelDescription>WAN Device on OpenWrt Router</modelDescription> \
			<modelName>WRT54G(S)</modelName> \
			<modelNumber>1.0</modelNumber> \
			<modelURL>http://www.linksys.com</modelURL> \
			<serialNumber>XXXXXXXXXX</serialNumber> \
			<UDN>uuid:75802409-bccb-40e7-8e6c-fa095ecce13e</UDN> \
			<UPC>Linux IGD</UPC> \
			<serviceList> \
			<service> \
			<serviceType>urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1</serviceType> \
			<serviceId>urn:upnp-org:serviceId:WANCommonIFC1</serviceId> \
			<controlURL>/upnp/control/WANCommonIFC1</controlURL> \
			<eventSubURL>/upnp/control/WANCommonIFC1</eventSubURL> \
			<SCPDURL>/gateicfgSCPD.xml</SCPDURL> \
			</service> \
			</serviceList> \
			<deviceList> \
			<device> \
			<deviceType>urn:schemas-upnp-org:device:WANConnectionDevice:1</deviceType> \
			<friendlyName>WANConnectionDevice</friendlyName> \
			<manufacturer>OpenWrt Project</manufacturer> \
			<manufacturerURL>http://www.openwrt.org</manufacturerURL> \
			<modelDescription>WanConnectionDevice on OpenWrt Router</modelDescription> \
			<modelName>WRT54G(S)</modelName> \
			<modelNumber>1.0</modelNumber> \
			<modelURL>http://www.linksys.com</modelURL> \
			<serialNumber>XXXXXXXXXX</serialNumber> \
			<UDN>uuid:75802409-bccb-40e7-8e6c-fa095ecce13e</UDN> \
			<UPC>Linux IGD</UPC> \
			<serviceList> \
			<service> \
			<serviceType>urn:schemas-upnp-org:service:WANIPConnection:1</serviceType> \
			<serviceId>urn:upnp-org:serviceId:WANIPConn1</serviceId> \
			<controlURL>/upnp/control/WANIPConn1</controlURL> \
			<eventSubURL>/upnp/control/WANIPConn1</eventSubURL> \
			<SCPDURL>/gateconnSCPD.xml</SCPDURL> \
			</service> \
			</serviceList> \
			</device> \
			</deviceList> \
			</device> \
			</deviceList> \
			<presentationURL>http://192.168.1.1/</presentationURL> \
			</device> \
			</root> ";


	UPnPParseDescriptionTest::UPnPParseDescriptionTest() : UnitTest("UPnPParseDescriptionTest")
	{}


	UPnPParseDescriptionTest::~UPnPParseDescriptionTest()
	{}

	bool UPnPParseDescriptionTest::doParse(const char* data,bool forward_test)
	{
		QString fn = "/tmp/UPnPParseDescriptionTest";
		QFile fptr(fn);
		if (!fptr.open(IO_WriteOnly))
		{
			Out() << "Cannot open " << fn << " : " << fptr.errorString() << endl;
			return false;
		}
		fptr.writeBlock(data,strlen(data));
		fptr.close();
		
		kt::UPnPRouter router(QString::null,"http://foobar.com");
		kt::UPnPDescriptionParser dp;
		
		if (!dp.parse(fn,&router))
		{
			bt::Delete(fn,true);
			return false;
		}
		else
		{
			Out() << "Succesfully parsed the UPnP contents" << endl;
			bt::Delete(fn,true);
			if (forward_test)
			{
				try
				{
					Out() << "Attempting to forward port 9999" << endl;
					router.forward(net::Port(9999,net::TCP,true));
				}
				catch (Error & e)
				{
					Out() << "Error forwarding : "<< e.toString() << endl;
					return false;
				}
			}
		//	router.debugPrintData();
			return true;
		}
	}

	bool UPnPParseDescriptionTest::doTest()
	{
		bool ret = true;
		if (!doParse(test_data1,false))
		{
			Out() << "Test data 1 failed" << endl;
			ret = false;
		}
		
		if (!doParse(test_data2,false))
		{
			Out() << "Test data 2 failed" << endl;
			ret = false;
		}
			
		if (!doParse(test_data3,false))
		{
			Out() << "Test data 3 failed" << endl;
			ret = false;
		}
		
		if (!doParse(test_data4,false))
		{
			Out() << "Test data 4 failed" << endl;
			ret = false;
		}
		
		return ret;
	}

}
