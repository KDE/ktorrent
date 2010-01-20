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
#include <utp/utpprotocol.h>

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
		bt::CircularBuffer wnd(20);
		QVERIFY(wnd.capacity() == 20);
		
		QVERIFY(wnd.write(data,13) == 13);
		QVERIFY(wnd.fill() == 13);
		
		QVERIFY(wnd.write(data2,6) == 6);
		QVERIFY(wnd.fill() == 19);
		
		QVERIFY(wnd.write(data2,6) == 1);
		QVERIFY(wnd.fill() == 20);
	}
	
	void testRead()
	{
		bt::CircularBuffer wnd(20);
		QVERIFY(wnd.capacity() == 20);
		QVERIFY(wnd.write(data,13) == 13);
		QVERIFY(wnd.write(data2,6) == 6);
		
		bt::Uint8 ret[19];
		QVERIFY(wnd.read(ret,19) == 19);
		QVERIFY(wnd.fill() == 0);
		QVERIFY(memcmp(ret,data,13) == 0);
		QVERIFY(memcmp(ret+13,data2,6) == 0);
		
		QVERIFY(wnd.write(data,13) == 13);
		QVERIFY(wnd.fill() == 13);
		
		QVERIFY(wnd.write(data2,6) == 6);
		QVERIFY(wnd.fill() == 19);
		
		QVERIFY(wnd.read(ret,19) == 19);
		QVERIFY(wnd.fill() == 0);
		QVERIFY(memcmp(ret,data,13) == 0);
		QVERIFY(memcmp(ret+13,data2,6) == 0);
	}
	
	void testLocalWindow()
	{
		bt::Uint8 wdata[1000];
		memset(wdata,0,1000);
		
		utp::Header hdr;
		utp::LocalWindow wnd(1000);
		wnd.setLastSeqNr(1);
		
		// write 500 bytes to it
		hdr.seq_nr = 2;
		QVERIFY(wnd.packetReceived(&hdr,wdata,500) == true);
		QVERIFY(wnd.availableSpace() == 500);
		QVERIFY(wnd.currentWindow() == 500);
		
		// write another 100 to it
		hdr.seq_nr++;
		QVERIFY(wnd.packetReceived(&hdr,wdata,100) == true);
		QVERIFY(wnd.availableSpace() == 400);
		QVERIFY(wnd.currentWindow() == 600);
		
		// read 300 from it
		QVERIFY(wnd.read(wdata,300) == 300);
		QVERIFY(wnd.availableSpace() == 700);
		QVERIFY(wnd.currentWindow() == 300);
	}
	
	void testPacketLoss()
	{
		bt::Uint8 wdata[1000];
		memset(wdata,0,1000);
		
		utp::Header hdr;
		utp::LocalWindow wnd(1000);
		wnd.setLastSeqNr(1);
		
		// write 500 bytes to it
		hdr.seq_nr = 2;
		QVERIFY(wnd.packetReceived(&hdr,wdata,500) == true);
		QVERIFY(wnd.availableSpace() == 500);
		QVERIFY(wnd.currentWindow() == 500);
		
		// write 100 bytes to it bit with the wrong sequence number
		hdr.seq_nr = 4;
		QVERIFY(wnd.packetReceived(&hdr,wdata,100) == true);
		QVERIFY(wnd.availableSpace() == 400);
		QVERIFY(wnd.currentWindow() == 600);
		QVERIFY(wnd.fill() == 500);
		
		// Try to read all of it, but we should only get back 500
		QVERIFY(wnd.read(wdata,600) == 500);
		QVERIFY(wnd.availableSpace() == 900);
		QVERIFY(wnd.currentWindow() == 100);
		QVERIFY(wnd.fill() == 0);
		
		// write the missing packet
		hdr.seq_nr = 3;
		QVERIFY(wnd.packetReceived(&hdr,wdata,100) == true);
		QVERIFY(wnd.availableSpace() == 800);
		QVERIFY(wnd.currentWindow() == 200);
		QVERIFY(wnd.fill() == 200);
	}
	
	void testPacketLoss2()
	{
		bt::Uint8 wdata[1000];
		memset(wdata,0,1000);
		
		utp::Header hdr;
		utp::LocalWindow wnd(1000);
		wnd.setLastSeqNr(0);
		
		// first write first and last packet
		bt::Uint32 step = 200;
		hdr.seq_nr = 1;
		QVERIFY(wnd.packetReceived(&hdr,wdata,step) == true);
		QVERIFY(wnd.availableSpace() == wnd.capacity() - step);
		QVERIFY(wnd.currentWindow() == step);
		hdr.seq_nr = 5;
		QVERIFY(wnd.packetReceived(&hdr,wdata,step) == true);
		QVERIFY(wnd.availableSpace() == wnd.capacity() - 2*step);
		QVERIFY(wnd.currentWindow() == 2*step);
		QVERIFY(wnd.fill() == step);
		
		// Now write 4
		hdr.seq_nr = 4;
		QVERIFY(wnd.packetReceived(&hdr,wdata,step) == true);
		QVERIFY(wnd.availableSpace() == wnd.capacity() - 3*step);
		QVERIFY(wnd.currentWindow() == 3*step);
		QVERIFY(wnd.fill() == step);
		
		// And then 3
		hdr.seq_nr = 3;
		QVERIFY(wnd.packetReceived(&hdr,wdata,step) == true);
		QVERIFY(wnd.availableSpace() == wnd.capacity() - 4*step);
		QVERIFY(wnd.currentWindow() == 4*step);
		QVERIFY(wnd.fill() == step);
		
		// And then 2
		hdr.seq_nr = 2;
		QVERIFY(wnd.packetReceived(&hdr,wdata,step) == true);
		QVERIFY(wnd.availableSpace() == wnd.capacity() - 5*step);
		QVERIFY(wnd.currentWindow() == 5*step);
		QVERIFY(wnd.fill() == 5*step);
	}
	
	void testToMuchData()
	{
		bt::Uint8 wdata[1000];
		memset(wdata,0,1000);
		
		utp::Header hdr;
		utp::LocalWindow wnd(500);
		wnd.setLastSeqNr(1);
		
		// write 500 bytes to it
		hdr.seq_nr = 2;
		QVERIFY(wnd.packetReceived(&hdr,wdata,500) == true);
		QVERIFY(wnd.availableSpace() == 0);
		QVERIFY(wnd.currentWindow() == 500);
		
		// writing more data should now have no effect at all
		hdr.seq_nr = 3;
		QVERIFY(wnd.packetReceived(&hdr,wdata,500) == false);
		QVERIFY(wnd.availableSpace() == 0);
		QVERIFY(wnd.currentWindow() == 500);
	}

private:
	bt::Uint8 data[13];
	bt::Uint8 data2[6];
};

QTEST_MAIN(LocalWindowTest)

#include "localwindowtest.moc"