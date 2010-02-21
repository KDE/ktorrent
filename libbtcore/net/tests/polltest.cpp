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
#include <util/pipe.h>
#include <net/poll.h>
#include <net/socket.h>

using namespace net;
using namespace bt;


class PollTest : public QEventLoop
{
	Q_OBJECT
public:
	
public slots:

	
private slots:
	void initTestCase()
	{
		bt::InitLog("polltest.log");
		
		
	}
	
	void cleanupTestCase()
	{
	}
	
	void testInput()
	{
		Poll p;
		Pipe pipe;
		
		QVERIFY(pipe.readerSocket() >= 0);
		QVERIFY(pipe.writerSocket() >= 0);
		QVERIFY(p.add(pipe.readerSocket(),Poll::INPUT) == 0);
		char test[] = "TEST";
		QVERIFY(pipe.write((const bt::Uint8*)test,4) == 4);
		QVERIFY(p.poll() == 1);
		QVERIFY(p.ready(0,net::Poll::INPUT));
		
		bt::Uint8 tmp[20];
		QVERIFY(pipe.read(tmp,20) == 4);
		QVERIFY(memcmp(tmp,test,4) == 0);
	}
	
	void testOutput()
	{
		Poll p;
		Pipe pipe;
		
		QVERIFY(pipe.readerSocket() >= 0);
		QVERIFY(pipe.writerSocket() >= 0);
		QVERIFY(p.add(pipe.writerSocket(),Poll::OUTPUT) == 0);
		QVERIFY(p.poll() == 1);
	}
	
	void testMultiplePolls()
	{
		Poll p;
		Pipe pipe;
		
		QVERIFY(pipe.readerSocket() >= 0);
		QVERIFY(pipe.writerSocket() >= 0);
		
		char test[] = "TEST";
		QVERIFY(pipe.write((const bt::Uint8*)test,4) == 4);
		
		for (int i = 0;i < 10;i++)
		{
			QVERIFY(p.add(pipe.readerSocket(),Poll::INPUT) == 0);
			QVERIFY(p.poll() == 1);
			QVERIFY(p.ready(0,net::Poll::INPUT));
			p.reset();
		}
		
		bt::Uint8 tmp[20];
		QVERIFY(pipe.read(tmp,20) == 4);
		QVERIFY(memcmp(tmp,test,4) == 0);
		QVERIFY(p.poll(100) == 0);
	}
	
	void testTimeout()
	{
		Poll p;
		Pipe pipe;
		
		QVERIFY(pipe.readerSocket() >= 0);
		QVERIFY(pipe.writerSocket() >= 0);
		QVERIFY(p.add(pipe.readerSocket(),Poll::INPUT) == 0);
		QVERIFY(p.poll(100) == 0);
	}
	
	void testSocket()
	{
		net::Socket sock(true,4);
		QVERIFY(sock.bind("127.0.0.1",0,true));
		
		net::Address local_addr = sock.getSockName();
		net::Socket writer(true,4);
		writer.setBlocking(false);
		writer.connectTo(local_addr);
		
		net::Address dummy;
		net::Poll poll;
		sock.prepare(&poll,net::Poll::INPUT);
		
		QVERIFY(poll.poll(1000) > 0);
		int fd = sock.accept(dummy);
		QVERIFY(fd >= 0);
		
		poll.reset();
		QVERIFY(writer.connectSuccesFull());
		
		net::Socket reader(fd,6);
		
		bt::Uint8 data[20];
		memset(data,0xFF,20);
		QVERIFY(writer.send(data,20) == 20);
		reader.prepare(&poll,net::Poll::INPUT);
		
		QVERIFY(poll.poll(1000) > 0);
		
		bt::Uint8 tmp[20];
		QVERIFY(reader.recv(tmp,20) == 20);
		QVERIFY(memcmp(tmp,data,20) == 0);
	}
	
private:
};

QTEST_MAIN(PollTest)

#include "polltest.moc"