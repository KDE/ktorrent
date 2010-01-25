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
#include <utp/connection.h>
#include <utp/utpsocket.h>
#include <utp/utpserver.h>
#include <util/functions.h>
#include <unistd.h>


#define BYTES_TO_SEND 1024*1024

using namespace utp;
using namespace bt;

class SendThread : public QThread
{
	Q_OBJECT
public:
	
	SendThread(Connection* outgoing,QObject* parent = 0) : QThread(parent),outgoing(outgoing)
	{}
	
	virtual void run()
	{
		QByteArray data(1024,0);
		int sent = 0;
		while (sent < BYTES_TO_SEND)
		{
			int to_send = 1024;
			int ret = outgoing->send((const bt::Uint8*)data.data(),to_send);
			if (ret > 0)
			{
				sent += ret;
			}
			else
			{
				msleep(50);
			}
		}
		
		sleep(2);
		Out(SYS_GEN|LOG_DEBUG) << "Transmitted " << sent << endl;
		outgoing->dumpStats();
	}
	
	Connection* outgoing;
};

class TransmitTest : public QEventLoop
{
	Q_OBJECT
public:
	TransmitTest(QObject* parent = 0) : QEventLoop(parent)
	{
	}
	
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
		bt::InitLog("transmittest.log");
		
		incoming = outgoing = 0;
		port = 50000;
		while (port < 60000)
		{
			if (!srv.changePort(port))
				port++;
			else
				break;
		}
		
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
	
	void testThreaded()
	{
		bt::Out(SYS_GEN|LOG_DEBUG) << "testThreaded" << bt::endl;
		if (outgoing->connectionState() != CS_CONNECTED || incoming->connectionState() != CS_CONNECTED)
		{
			QSKIP("Not Connected",SkipAll);
			return;
		}
		
		SendThread st(outgoing);
		st.start(); // The thread will start sending a whole bunch of data
		int received = 0;
		while (received < BYTES_TO_SEND)
		{
			bt::Uint32 ba = incoming->bytesAvailable();
			if (ba > 0)
			{
				QByteArray data(ba,0);
				int ret = incoming->recv((bt::Uint8*)data.data(),ba);
				if (ret > 0)
					received += ret;
			}
			else
			{
				usleep(50000);
				if (st.isFinished())
					break;
			}
		}
		
		st.wait();
		Out(SYS_GEN|LOG_DEBUG) << "Received " << received << endl;
		incoming->dumpStats();
		QVERIFY(incoming->bytesAvailable() == 0);
		QVERIFY(outgoing->allDataSent());
		QVERIFY(received >= BYTES_TO_SEND);
		
	}
	
private:
	
	
private:
	Connection* incoming;
	Connection* outgoing;
	utp::UTPServer srv;
	int port;
};

QTEST_MAIN(TransmitTest)

#include "transmittest.moc"