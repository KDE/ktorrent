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

#include <QtTest>
#include <QObject>
#include <utp/localwindow.h>

using namespace utp;

class LocalWindowTest : public QObject
{
	Q_OBJECT
public:
	
private slots:
	void init()
	{
		memset(data,0xFF,13);
		memset(data2,0xEE,6);
	}
	
	void testWrite()
	{
		LocalWindow wnd(20);
		QVERIFY(wnd.maxWindow() == 20);
		
		QVERIFY(wnd.write(data,13) == 13);
		QVERIFY(wnd.currentWindow() == 13);
		
		QVERIFY(wnd.write(data2,6) == 6);
		QVERIFY(wnd.currentWindow() == 19);
		
		QVERIFY(wnd.write(data2,6) == 1);
		QVERIFY(wnd.currentWindow() == 20);
	}
	
	void testRead()
	{
		LocalWindow wnd(20);
		QVERIFY(wnd.maxWindow() == 20);
		QVERIFY(wnd.write(data,13) == 13);
		QVERIFY(wnd.write(data2,6) == 6);
		
		bt::Uint8 ret[19];
		QVERIFY(wnd.read(ret,19) == 19);
		QVERIFY(wnd.currentWindow() == 0);
		QVERIFY(memcmp(ret,data,13) == 0);
		QVERIFY(memcmp(ret+13,data2,6) == 0);
		
		QVERIFY(wnd.write(data,13) == 13);
		QVERIFY(wnd.currentWindow() == 13);
		
		QVERIFY(wnd.write(data2,6) == 6);
		QVERIFY(wnd.currentWindow() == 19);
		
		QVERIFY(wnd.read(ret,19) == 19);
		QVERIFY(wnd.currentWindow() == 0);
		QVERIFY(memcmp(ret,data,13) == 0);
		QVERIFY(memcmp(ret+13,data2,6) == 0);
	}

private:
	bt::Uint8 data[13];
	bt::Uint8 data2[6];
};

QTEST_MAIN(LocalWindowTest)

#include "localwindowtest.moc"