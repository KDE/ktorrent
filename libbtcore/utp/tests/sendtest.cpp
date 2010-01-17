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

using namespace utp;

class SendTest : public QEventLoop
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
		bt::InitLog("sendtest.log");
		
		incoming = 0;
		port = 50000;
		while (port < 60000)
		{
			if (!bt::Globals::instance().initUTPServer(port))
				port++;
			else
				break;
		}
	}
	
	void cleanupTestCase()
	{
		delete incoming;
		delete outgoing;
		bt::Globals::instance().shutdownUTPServer();
	}
	
	void testConnect()
	{
		net::Address addr("127.0.0.1",port);
		utp::UTPServer & srv = bt::Globals::instance().getUTPServer();
		connect(&srv,SIGNAL(accepted(Connection*)),this,SLOT(accepted(Connection*)),Qt::QueuedConnection);
		outgoing = new utp::UTPSocket();
		outgoing->setBlocking(false);
		outgoing->connectTo(addr);
		
		QTimer::singleShot(5000,this,SLOT(endEventLoop())); // use a 5 second timeout
		exec();
		QVERIFY(incoming != 0);
	}
	
	void testSend()
	{
		outgoing->setBlocking(true);
		char* test = "TEST";
		
		outgoing->send((const bt::Uint8*)test,strlen(test));
		
		char tmp[20];
		memset(tmp,0,20);
		int ret = incoming->recv((bt::Uint8*)tmp,20);
		QVERIFY(ret == 4);
		QVERIFY(memcmp(tmp,test,ret) == 0);
	}
	
	void testSend2()
	{
		bt::Uint8* sdata = new bt::Uint8[1000];
		outgoing->send(sdata,1000);
		
		bt::Uint8* rdata = new bt::Uint8[1000];
		int ret = incoming->recv(rdata,1000);
		QVERIFY(ret == 1000);
		QVERIFY(memcmp(sdata,rdata,ret) == 0);
		
		delete[] rdata;
		delete[] sdata;
	}
	
	void testSend3()
	{
		char* test = "TEST";
		
		outgoing->send((const bt::Uint8*)test,strlen(test));
		incoming->send((const bt::Uint8*)test,strlen(test));
		
		char tmp[20];
		memset(tmp,0,20);
		int ret = incoming->recv((bt::Uint8*)tmp,20);
		QVERIFY(ret == 4);
		QVERIFY(memcmp(tmp,test,ret) == 0);
		
		memset(tmp,0,20);
		ret = outgoing->recv((bt::Uint8*)tmp,20);
		QVERIFY(ret == 4);
		QVERIFY(memcmp(tmp,test,ret) == 0);
	}
	
private:
	int port;
	utp::UTPSocket* incoming;
	utp::UTPSocket* outgoing;
};

QTEST_MAIN(SendTest)

#include "sendtest.moc"