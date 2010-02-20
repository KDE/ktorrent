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
#include <unistd.h>
#include <util/log.h>
#include <utp/utpserver.h>
#include <utp/utpsocket.h>
#include <net/poll.h>
#include <torrent/globals.h>

using namespace utp;
using namespace net;
using namespace bt;

#define NUM_SOCKETS 20

class UTPPollTest : public QEventLoop
{
	Q_OBJECT
public:
	
	
public slots:
	void accepted(Connection* conn)
	{
		incoming[num_accepted++] = new UTPSocket(conn);
		Out(SYS_GEN|LOG_DEBUG) << "Accepted " << num_accepted << endl;
		if (num_accepted >= NUM_SOCKETS)
			exit();
	}
	
	void endEventLoop()
	{
		exit();
	}
	
private slots:
	void initTestCase()
	{
		bt::InitLog("utppolltest.log");
		
		port = 50000;
		while (port < 60000)
		{
			if (!bt::Globals::instance().initUTPServer(port))
				port++;
			else
				break;
		}
		
		bt::Globals::instance().getUTPServer().setCreateSockets(false);
		num_accepted = 0;
		for (int i = 0;i < NUM_SOCKETS;i++)
		{
			outgoing[i] = incoming[i] = 0;
		}
	}
	
	void cleanupTestCase()
	{
		bt::Globals::instance().shutdownUTPServer();
	}
	
	void testPollConnect()
	{
		poller.reset();
		Out(SYS_GEN|LOG_DEBUG) << "testPollConnect " << endl;
		utp::UTPSocket s;
		s.setBlocking(false);
		s.connectTo(net::Address("127.0.0.1",port));
		s.prepare(&poller,Poll::OUTPUT);
		QVERIFY(poller.poll() > 0);
		QVERIFY(s.ready(&poller,Poll::OUTPUT));
		QVERIFY(s.connectSuccesFull());
		poller.reset();
	}

	void testConnect()
	{
		Out(SYS_GEN|LOG_DEBUG) << "testConnect " << endl;
		for (int i = 0;i < NUM_SOCKETS;i++)
		{
			outgoing[i] = new UTPSocket(); 
			outgoing[i]->setBlocking(false);
			outgoing[i]->connectTo(net::Address("127.0.0.1",port));
		}
		
		utp::UTPServer & srv = bt::Globals::instance().getUTPServer();
		connect(&srv,SIGNAL(accepted(Connection*)),this,SLOT(accepted(Connection*)),Qt::QueuedConnection);
		
		QTimer::singleShot(5000,this,SLOT(endEventLoop())); // use a 5 second timeout
		exec();
		QVERIFY(num_accepted == NUM_SOCKETS);
	}
	
	void testPollInput()
	{
		Out(SYS_GEN|LOG_DEBUG) << "testPollInput " << endl;
		char test[] = "test\n";
		
		for (int j = 0;j < 10;j++)
		{
			for (int i = 0;i < NUM_SOCKETS;i++)
			{
				outgoing[i]->send((const bt::Uint8*)test,strlen(test));
				incoming[i]->prepare(&poller,Poll::INPUT);
			}
			
			sleep(1);
			QVERIFY(poller.poll(1000) > 0);
			for (int i = 0;i < NUM_SOCKETS;i++)
			{
				QVERIFY(incoming[i]->ready(&poller,net::Poll::INPUT));
				bt::Uint8 tmp[20];
				QVERIFY(incoming[i]->recv(tmp,20) == strlen(test));
				QVERIFY(memcmp(tmp,test,strlen(test)) == 0);
			}
			poller.reset();
		}
	}
	
	void testPollOutput()
	{
		Out(SYS_GEN|LOG_DEBUG) << "testPollOutput " << endl;
		for (int i = 0;i < NUM_SOCKETS;i++)
		{
			incoming[i]->prepare(&poller,Poll::OUTPUT);
		}
		
		int ret = poller.poll();
		Out(SYS_GEN|LOG_DEBUG) << "ret = " << ret << endl;
		for (int i = 0;i < NUM_SOCKETS;i++)
		{
			QVERIFY(incoming[i]->ready(&poller,Poll::OUTPUT));
		}
		
		QVERIFY(ret > 0);
		poller.reset();
	}
	
	void testPollClose()
	{
		Out(SYS_GEN|LOG_DEBUG) << "testPollClose " << endl;
		for (int i = 0;i < NUM_SOCKETS;i++)
		{
			incoming[i]->close();
			outgoing[i]->prepare(&poller,net::Poll::INPUT);
		}
		
		QVERIFY(poller.poll() > 0);
		poller.reset();
	}

private:
	int port;
	utp::UTPSocket* outgoing[NUM_SOCKETS];
	utp::UTPSocket* incoming[NUM_SOCKETS];
	int num_accepted;
	Poll poller;
};

QTEST_MAIN(UTPPollTest)

#include "utppolltest.moc"