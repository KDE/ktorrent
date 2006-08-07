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
#include <plugins/upnp/upnprouter.h>
#include <plugins/upnp/upnpdescriptionparser.h>
#include <plugins/upnp/upnpmcastsocket.h>
#include "upnpparseresponsetest.h"

using namespace kt;
using namespace bt;

namespace utest
{

	UPnPParseResponseTest::UPnPParseResponseTest() : UnitTest("UPnPParseResponseTest")
	{}


	UPnPParseResponseTest::~UPnPParseResponseTest()
	{}


	bool UPnPParseResponseTest::doTest()
	{
		static const char* test_ps = "M-SEARCH * HTTP/1.1\r\n"
				"HOST: 239.255.255.250:1900\r\n"
				"ST:urn:schemas-upnp-org:device:InternetGatewayDevice:1\r\n"
				"MAN:\"ssdp:discover\"\r\n"
				"MX:3\r\n"
				"HTTP/1.1 200 OK\r\n"
				"CACHE-CONTROL: max-age=1800\r\n"
				"DATE: Mon, 13 Mar 2006 19:55:10 GMT \r\n"
				"EXT:\r\n"
				"LOCATION: http://192.168.1.1:52869/gatedesc.xml\r\n"
				"SERVER: Linux/2.4.17_mvl21-malta-mips_fp_le, UPnP/1.0, Intel SDK for UPnP devices /1.2\r\n"
				"ST: urn:schemas-upnp-org:device:InternetGatewayDevice:1\r\n"
				"USN: uuid:75802409-bccb-40e7-8e6c-fa095ecce13e::urn:schemas-upnp-org:device:InternetGatewayDevice:1\r\n\r\n";
		
		UPnPMCastSocket mcast;
		UPnPRouter* r = mcast.parseResponse(QCString(test_ps));
		if (r)
		{
			delete r;
			return true;
		}
		
		return false;
	}

}
