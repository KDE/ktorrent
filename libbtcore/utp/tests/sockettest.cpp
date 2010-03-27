/***************************************************************************
*   Copyright (C) 2010 by Joris Guisson                                   *
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

#include <QtTest>
#include <QObject>
#include <util/log.h>
#include <utp/utpserver.h>
#include <utp/utpsocket.h>
#include <torrent/globals.h>
#include <boost/concept_check.hpp>

using namespace utp;

class SocketTest : public QEventLoop
{
	Q_OBJECT
public:
	
public slots:
	void accepted(Connection* conn)
	{
		incoming = new UTPSocket(conn);
		exit();
	}
	
	void endEventLoop()
	{
		exit();
	}
	
private slots:
	void initTestCase()
	{
		bt::InitLog("sockettest.log");
		incoming = outgoing = 0;
		
		port = 50000;
		while (port < 60000)
		{
			if (bt::Globals::instance().initUTPServer(port))
				break;
			else
				port++;
		}
		
		bt::Globals::instance().getUTPServer().setCreateSockets(false);
	}
	
	void cleanupTestCase()
	{
		bt::Globals::instance().shutdownUTPServer();
		delete incoming;
		delete outgoing;
	}
	
	void testConnect()
	{
		UTPServer & srv = bt::Globals::instance().getUTPServer();
		net::Address addr("127.0.0.1",port);
		connect(&srv,SIGNAL(accepted(Connection*)),this,SLOT(accepted(Connection*)),Qt::QueuedConnection);
		outgoing = new UTPSocket();
		outgoing->connectTo(addr);
		QTimer::singleShot(5000,this,SLOT(endEventLoop())); // use a 5 second timeout
		exec();
		QVERIFY(incoming != 0);
		QVERIFY(incoming->connectSuccesFull());
		QVERIFY(outgoing->connectSuccesFull());
	}
	
	void testSend()
	{
		outgoing->setBlocking(true);
		incoming->setBlocking(true);
		char test[] = "TEST";
		
		UTPSocket* a = incoming;
		UTPSocket* b = outgoing;
		for (int i = 0;i < 10;i++)
		{
			int ret = a->send((const bt::Uint8*)test,strlen(test));
			QVERIFY(ret == strlen(test));
			
			char tmp[20];
			memset(tmp,0,20);
			ret = b->recv((bt::Uint8*)tmp,20);
			QVERIFY(ret == 4);
			QVERIFY(memcmp(tmp,test,ret) == 0);
			std::swap(a,b);
		}
	}
	
	void testClose()
	{
		outgoing->setBlocking(true);
		incoming->close();
		bt::Uint8 tmp[20];
		int ret = outgoing->recv(tmp,20);
		QVERIFY(ret == 0);
	}
	
	void testConnectionTimeout()
	{
		UTPSocket sock;
		net::Address addr("127.0.0.1",port + 1);
		sock.setBlocking(true);
		QVERIFY(sock.connectTo(addr) == false);
	}
	
	void testInvalidAddress()
	{
		UTPSocket sock;
		net::Address addr("127.0.0.1",0);
		sock.setBlocking(true);
		QVERIFY(sock.connectTo(addr) == false);
	}
	
private:
	int port;
	utp::UTPSocket* incoming;
	utp::UTPSocket* outgoing;
};

QTEST_MAIN(SocketTest)

#include "sockettest.moc"