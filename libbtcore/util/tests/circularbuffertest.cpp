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
#include <time.h>
#include <util/log.h>
#include <util/pipe.h>
#include <util/circularbuffer.h>

using namespace bt;


class CircularBufferTest : public QEventLoop
{
	Q_OBJECT
public:

	
private slots:
	void initTestCase()
	{
		memset(data,0xFF,13);
		memset(data2,0xEE,6);
	}
	
	void cleanupTestCase()
	{
	}
	
	void testWrite()
	{
		bt::CircularBuffer wnd(20);
		QVERIFY(wnd.capacity() == 20);
		QVERIFY(wnd.size() == 0);
		
		QVERIFY(wnd.write(data,13) == 13);
		QVERIFY(wnd.size() == 13);
		
		QVERIFY(wnd.write(data2,6) == 6);
		QVERIFY(wnd.size() == 19);
		
		QVERIFY(wnd.write(data2,6) == 1);
		QVERIFY(wnd.size() == 20);
	}
	
	void testRead()
	{
		bt::CircularBuffer wnd(20);
		QVERIFY(wnd.capacity() == 20);
		QVERIFY(wnd.write(data,13) == 13);
		QVERIFY(wnd.write(data2,6) == 6);
		
		bt::Uint8 ret[19];
		QVERIFY(wnd.read(ret,19) == 19);
		QVERIFY(wnd.size() == 0);
		QVERIFY(memcmp(ret,data,13) == 0);
		QVERIFY(memcmp(ret+13,data2,6) == 0);
		
		QVERIFY(wnd.write(data,13) == 13);
		QVERIFY(wnd.size() == 13);
		
		QVERIFY(wnd.write(data2,6) == 6);
		QVERIFY(wnd.size() == 19);
		
		QVERIFY(wnd.read(ret,19) == 19);
		QVERIFY(wnd.size() == 0);
		QVERIFY(memcmp(ret,data,13) == 0);
		QVERIFY(memcmp(ret+13,data2,6) == 0);
	}
	
	void testIntensively()
	{
		CircularBuffer cbuf(20);
		qsrand(time(0));
		
		for (int i = 0;i < 1000;i++)
		{
			Uint32 r = 1 + qrand() % 20;
			Uint32 expected = r;
			if (expected + cbuf.size() >= 20)
				expected = 20 - cbuf.size();
			
			QVERIFY(cbuf.write(data,r) == expected);
			
			bt::Uint8 ret[20];
			memset(ret,0,20);
			r = 1 + qrand() % 20;
			expected = qMin(r,cbuf.size());
			QVERIFY(cbuf.read(ret,expected) == expected);
		}
	}
	
	void testErrors()
	{
		CircularBuffer cbuf(20);
		bt::Uint8 too_much[40];
		QVERIFY(cbuf.write(too_much,40) == 20);
		QVERIFY(cbuf.write(too_much,40) == 0);
	}
	
	
private:
	bt::Uint8 data[13];
	bt::Uint8 data2[6];
};

QTEST_MAIN(CircularBufferTest)

#include "circularbuffertest.moc"