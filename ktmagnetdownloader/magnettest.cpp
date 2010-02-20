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

#include "magnettest.h"

#include <QApplication>
#include <dht/dhtbase.h>
#include <magnet/magnetlink.h>
#include <torrent/globals.h>
#include <interfaces/functions.h>
#include <settings.h>
#include <libktupnp/upnpmcastsocket.h>
#include <QTimer>
#include <QNetworkInterface>
#include <util/functions.h>
#include <util/log.h>
#include <util/error.h>
#include <torrent/server.h>
#include <QFile>
#include <bcodec/bencoder.h>
#include <peer/authenticationmonitor.h>

using namespace kt;
using namespace bt;

MagnetTest::MagnetTest(const bt::MagnetLink & mlink, QObject* parent) : QObject(parent),mlink(mlink)
{
	upnp = new kt::UPnPMCastSocket();
	connect(upnp,SIGNAL(discovered(kt::UPnPRouter*)),this,SLOT(routerDiscovered(kt::UPnPRouter*)));
	
	mdownloader = new MagnetDownloader(mlink,this);
	connect(mdownloader,SIGNAL(foundMetaData(bt::MagnetDownloader*,QByteArray)),
			this,SLOT(foundMetaData(bt::MagnetDownloader*,QByteArray)));
			
	QTimer::singleShot(0,this,SLOT(start()));
	connect(&timer,SIGNAL(timeout()),this,SLOT(update()));
}

MagnetTest::~MagnetTest()
{
	delete upnp;
}


void MagnetTest::routerDiscovered(kt::UPnPRouter* router)
{
	net::Port port;
	port.number = Settings::dhtPort();
	port.proto = net::UDP;
	port.forward = true;
	router->forward(port);
}

void MagnetTest::start()
{
	Uint16 port = Settings::port();
	if (port == 0)
	{
		port = 6881;
		Settings::setPort(6881);
	}
	
	// Make sure network interface is set properly before server is initialized
	if (Settings::networkInterface() != 0)
	{
		QList<QNetworkInterface> iface_list = QNetworkInterface::allInterfaces();
		int iface = Settings::networkInterface();
		if (iface > iface_list.count())
			SetNetworkInterface(QString::null);
		else
			SetNetworkInterface(iface_list[iface - 1].name());
	}
	
	
	Uint16 i = 0;
	do
	{
		Globals::instance().initTCPServer(port + i);
		i++;
	}
	while (!Globals::instance().getTCPServer().isOK() && i < 10);
	
	if (Globals::instance().getTCPServer().isOK())
	{
		Out(SYS_GEN|LOG_NOTICE) << "Bound to port " << (port + i - 1) << endl;
	}
	else
	{
		Out(SYS_GEN|LOG_IMPORTANT) << "Cannot find free port" << endl;
	}
	
	// start DHT
	bt::Globals::instance().getDHT().start(kt::DataDir() + "dht_table",kt::DataDir() + "dht_key",Settings::dhtPort());
	
	// Start UPnP router discovery
	upnp->loadRouters(kt::DataDir() + "routers");
	upnp->discover();
	
	mdownloader->start();
	timer.start(500);
}

void MagnetTest::update()
{
	try
	{
		bt::AuthenticationMonitor::instance().update();
		mdownloader->update();
	}
	catch (bt::Error & err)
	{
		Out(SYS_GEN|LOG_IMPORTANT) << "Caught bt::Error: " << err.toString() << endl;
	}
}


void MagnetTest::foundMetaData(MagnetDownloader* md, const QByteArray& data)
{
	Q_UNUSED(md);
	Out(SYS_GEN|LOG_IMPORTANT) << "Saving to output.torrent" << endl;
	bt::File fptr;
	if (fptr.open("output.torrent","wb"))
	{
		BEncoder enc(&fptr);
		enc.beginDict();
		if (!mlink.tracker().isEmpty())
		{
			enc.write("announce");
			enc.write(mlink.tracker());
		}
		enc.write("info");
		fptr.write(data.data(),data.size());
		enc.end();
		QTimer::singleShot(0,qApp,SLOT(quit()));
	}
	else
	{
		Out(SYS_GEN|LOG_IMPORTANT) << "Failed to open output.torrent: " << fptr.errorString() << endl;
	}
}

