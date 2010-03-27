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
#include <utp/connection.h>

using namespace utp;

class FinTest : public QEventLoop
{
	Q_OBJECT
public:
	
public slots:
	void accepted(Connection* conn)
	{
		incoming = conn;
		exit();
	}
	
	void endEventLoop()
	{
		exit();
	}
	
private slots:
	void initTestCase()
	{
		bt::InitLog("fintest.log");
		
		incoming = outgoing = 0;
		port = 50000;
		while (port < 60000)
		{
			if (!srv.changePort(port))
				port++;
			else
				break;
		}
		
		srv.setCreateSockets(false);
		srv.start();
	}
	
	void cleanupTestCase()
	{
		srv.stop();
	}
	
	void testConnect()
	{
		net::Address addr("127.0.0.1",port);
		connect(&srv,SIGNAL(accepted(Connection*)),this,SLOT(accepted(Connection*)),Qt::QueuedConnection);
		outgoing = srv.connectTo(addr);
		QVERIFY(outgoing != 0);
		QTimer::singleShot(5000,this,SLOT(endEventLoop())); // use a 5 second timeout
		exec();
		QVERIFY(incoming != 0);
	}
	
	void testFin()
	{
		bt::Out(SYS_UTP|LOG_DEBUG) << "testFin" << bt::endl;
		if (outgoing->connectionState() != CS_CONNECTED || incoming->connectionState() != CS_CONNECTED)
		{
			QSKIP("Not Connected",SkipAll);
			return;
		}
		
		char test[] = "This is the fin test";
		outgoing->send((const bt::Uint8*)test,strlen(test));
		if (incoming->waitForData())
		{
			bt::Uint8 tmp[100];
			memset(tmp,0,100);
			int ret = incoming->recv(tmp,100);
			QVERIFY(ret == strlen(test));
			QVERIFY(memcmp(tmp,test,strlen(test)) == 0);
			
			outgoing->close();
			if (incoming->connectionState() != CS_CLOSED)
				incoming->waitForData();
			
			// connection should be closed now
			ret = incoming->recv(tmp,100);
			QVERIFY(incoming->connectionState() == CS_CLOSED);
			QVERIFY(ret == -1);
		}
		else
			QFAIL("No data received");
	}
	
private:
	utp::UTPServer srv;
	int port;
	Connection* incoming;
	Connection* outgoing;
};

QTEST_MAIN(FinTest)

#include "fintest.moc"