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
#include <util/log.h>
#include <utp/localwindow.h>
#include <utp/utpprotocol.h>

using namespace utp;
using namespace bt;


class LocalWindowTest : public QObject
{
	Q_OBJECT
public:
	
private slots:
	void initTestCase()
	{
		bt::InitLog("localwindowtest.log");
	}
	
	
	
	void testLocalWindow()
	{
		Out(SYS_UTP|LOG_DEBUG) << "testLocalWindow" << endl;
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
		Out(SYS_UTP|LOG_DEBUG) << "testPacketLoss" << endl;
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
		QVERIFY(wnd.size() == 500);
		
		// Try to read all of it, but we should only get back 500
		QVERIFY(wnd.read(wdata,600) == 500);
		QVERIFY(wnd.availableSpace() == 900);
		QVERIFY(wnd.currentWindow() == 100);
		QVERIFY(wnd.size() == 0);
		
		// write the missing packet
		hdr.seq_nr = 3;
		QVERIFY(wnd.packetReceived(&hdr,wdata,100) == true);
		QVERIFY(wnd.availableSpace() == 800);
		QVERIFY(wnd.currentWindow() == 200);
		QVERIFY(wnd.size() == 200);
	}
	
	void testPacketLoss2()
	{
		Out(SYS_UTP|LOG_DEBUG) << "testPacketLoss2" << endl;
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
		QVERIFY(wnd.size() == step);
		
		// Now write 4
		hdr.seq_nr = 4;
		QVERIFY(wnd.packetReceived(&hdr,wdata,step) == true);
		QVERIFY(wnd.availableSpace() == wnd.capacity() - 3*step);
		QVERIFY(wnd.currentWindow() == 3*step);
		QVERIFY(wnd.size() == step);
		
		// And then 3
		hdr.seq_nr = 3;
		QVERIFY(wnd.packetReceived(&hdr,wdata,step) == true);
		QVERIFY(wnd.availableSpace() == wnd.capacity() - 4*step);
		QVERIFY(wnd.currentWindow() == 4*step);
		QVERIFY(wnd.size() == step);
		
		// And then 2
		hdr.seq_nr = 2;
		QVERIFY(wnd.packetReceived(&hdr,wdata,step) == true);
		QVERIFY(wnd.availableSpace() == wnd.capacity() - 5*step);
		QVERIFY(wnd.currentWindow() == 5*step);
		QVERIFY(wnd.size() == 5*step);
	}
	
	void testToMuchData()
	{
		Out(SYS_UTP|LOG_DEBUG) << "testToMuchData" << endl;
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
	
	void testSelectiveAck()
	{
		Out(SYS_UTP|LOG_DEBUG) << "testSelectiveAck" << endl;
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
		QVERIFY(wnd.size() == step);
		
		// Check SelectiveAck generation
		QVERIFY(wnd.selectiveAckBits() == 3);
		bt::Uint8 sack_data[6];
		SelectiveAck sack;
		sack.length = 4;
		sack.extension = 0;
		sack.bitmask = sack_data + 2;
		wnd.fillSelectiveAck(&sack);
		QVERIFY(sack_data[2] == 0x4);
		QVERIFY(sack_data[3] == 0x0);
		QVERIFY(sack_data[4] == 0x0);
		QVERIFY(sack_data[5] == 0x0);
		
		// Now write 4
		hdr.seq_nr = 4;
		QVERIFY(wnd.packetReceived(&hdr,wdata,step) == true);
		QVERIFY(wnd.availableSpace() == wnd.capacity() - 3*step);
		QVERIFY(wnd.currentWindow() == 3*step);
		QVERIFY(wnd.size() == step);
		
		// Check selective ack again
		QVERIFY(wnd.selectiveAckBits() == 3);
		sack.length = 4;
		sack.extension = 0;
		wnd.fillSelectiveAck(&sack);
		QVERIFY(sack_data[2] == 0x6);
		QVERIFY(sack_data[3] == 0x0);
		QVERIFY(sack_data[4] == 0x0);
		QVERIFY(sack_data[5] == 0x0);
		
		// Now write 3
		hdr.seq_nr = 3;
		QVERIFY(wnd.packetReceived(&hdr,wdata,step) == true);
		QVERIFY(wnd.availableSpace() == wnd.capacity() - 4*step);
		QVERIFY(wnd.currentWindow() == 4*step);
		QVERIFY(wnd.size() == step);
		
		// Check selective ack again
		QVERIFY(wnd.selectiveAckBits() == 3);
		sack.length = 4;
		sack.extension = 0;
		wnd.fillSelectiveAck(&sack);
		QVERIFY(sack_data[2] == 0x7);
		QVERIFY(sack_data[3] == 0x0);
		QVERIFY(sack_data[4] == 0x0);
		QVERIFY(sack_data[5] == 0x0);
		
		// And then 2
		hdr.seq_nr = 2;
		QVERIFY(wnd.packetReceived(&hdr,wdata,step) == true);
		QVERIFY(wnd.availableSpace() == wnd.capacity() - 5*step);
		QVERIFY(wnd.currentWindow() == 5*step);
		QVERIFY(wnd.size() == 5*step);
		// selective ack should now be unnecessary
		QVERIFY(wnd.selectiveAckBits() == 0);
	}

private:
	//bt::Uint8 data[13];
	//bt::Uint8 data2[6];
};

QTEST_MAIN(LocalWindowTest)

#include "localwindowtest.moc"