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
	
private:
};

QTEST_MAIN(PollTest)

#include "polltest.moc"