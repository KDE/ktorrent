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
#include <utp/remotewindow.h>
#include <utp/connection.h>
#include <util/functions.h>

using namespace utp;

class RemoteWindowTest : public QObject,public utp::Retransmitter
{
	Q_OBJECT
public:
	QSet<bt::Uint16> retransmit_seq_nr;
	bool update_rtt_called;
	bool retransmit_ok;
	
	
	RemoteWindowTest(QObject* parent = 0) : QObject(parent),update_rtt_called(false),retransmit_ok(false)
	{
	}
	
	virtual void updateRTT(const Header* hdr,bt::Uint32 packet_rtt,bt::Uint32 packet_size)
	{
		Q_UNUSED(hdr);
		Q_UNUSED(packet_rtt);
		Q_UNUSED(packet_size);
		update_rtt_called = true;
	}
	
	virtual void retransmit(const QByteArray & packet,bt::Uint16 p_seq_nr)
	{
		bt::Out(SYS_UTP|LOG_NOTICE) << "retransmit " << p_seq_nr << bt::endl;
		retransmit_ok = retransmit_seq_nr.contains(p_seq_nr);
	}
	
	void reset()
	{
		retransmit_seq_nr.clear();
		update_rtt_called = false;
		retransmit_ok = false;
	}
	
	virtual bt::Uint32 currentTimeout() const {return 1000;}
		
private slots:
	void initTestCase()
	{
		bt::InitLog("remotewindowtest.log");
	}
	
	void cleanupTestCase()
	{
	}
	
	void init()
	{
		reset();
	}
	
	void testNormalUsage()
	{
		QByteArray pkt(200,0);
		
		RemoteWindow wnd;
		wnd.addPacket(pkt,1,bt::Now());
		QVERIFY(!wnd.allPacketsAcked());
		QVERIFY(wnd.numUnackedPackets() == 1);
		
		wnd.addPacket(pkt,2,bt::Now());
		QVERIFY(wnd.numUnackedPackets() == 2);
		
		wnd.addPacket(pkt,3,bt::Now());
		QVERIFY(wnd.numUnackedPackets() == 3);
		
		Header hdr;
		hdr.ack_nr = 1;
		hdr.wnd_size = 5000;
		wnd.packetReceived(&hdr,0,this);
		QVERIFY(wnd.numUnackedPackets() == 2);
		QVERIFY(update_rtt_called);
		
		reset();
		hdr.ack_nr = 2;
		wnd.packetReceived(&hdr,0,this);
		QVERIFY(wnd.numUnackedPackets() == 1);
		QVERIFY(update_rtt_called);
		
		reset();
		hdr.ack_nr = 3;
		wnd.packetReceived(&hdr,0,this);
		QVERIFY(wnd.numUnackedPackets() == 0);
		QVERIFY(update_rtt_called);
	}
		
	void testMultipleAcks()
	{
		QByteArray pkt(200,0);
		
		RemoteWindow wnd;
		wnd.addPacket(pkt,1,bt::Now());
		QVERIFY(!wnd.allPacketsAcked());
		QVERIFY(wnd.numUnackedPackets() == 1);
		
		wnd.addPacket(pkt,2,bt::Now());
		QVERIFY(wnd.numUnackedPackets() == 2);
		
		wnd.addPacket(pkt,3,bt::Now());
		QVERIFY(wnd.numUnackedPackets() == 3);
		
		Header hdr;
		hdr.ack_nr = 3;
		hdr.wnd_size = 5000;
		wnd.packetReceived(&hdr,0,this);
		QVERIFY(wnd.numUnackedPackets() == 0);
		QVERIFY(update_rtt_called);
	}
	
	void testSelectiveAck()
	{
		QByteArray pkt(200,0);
		
		RemoteWindow wnd;
		wnd.addPacket(pkt,1,bt::Now());
		QVERIFY(!wnd.allPacketsAcked());
		QVERIFY(wnd.numUnackedPackets() == 1);
		
		wnd.addPacket(pkt,2,bt::Now());
		QVERIFY(wnd.numUnackedPackets() == 2);
		
		wnd.addPacket(pkt,3,bt::Now());
		QVERIFY(wnd.numUnackedPackets() == 3);
		
		// Selectively ack 3
		bt::Uint8 sack_data[6];
		memset(sack_data,0,6);
		SelectiveAck sack;;
		sack.length = 4;
		sack.extension = 0;
		sack.bitmask = sack_data + 2;
		Ack(&sack,3);
		
		Header hdr;
		hdr.ack_nr = 0;
		hdr.wnd_size = 5000;
		wnd.packetReceived(&hdr,&sack,this);
		QVERIFY(wnd.numUnackedPackets() == 2);
		QVERIFY(update_rtt_called);
		
		reset();
		
		// Ack the rest
		hdr.ack_nr = 3;
		hdr.wnd_size = 5000;
		wnd.packetReceived(&hdr,0,this);
		QVERIFY(wnd.numUnackedPackets() == 0);
		QVERIFY(update_rtt_called);
	}
	
	void testRetransmits()
	{
		QByteArray pkt(200,0);
		
		RemoteWindow wnd;
		for (bt::Uint32 i = 0;i < 4;i++)
		{
			wnd.addPacket(pkt,i+1,bt::Now());
			QVERIFY(!wnd.allPacketsAcked());
			QVERIFY(wnd.numUnackedPackets() == i + 1);
		}
		
		// Selectively ack the last 3 packets
		bt::Uint8 sack_data[6];
		memset(sack_data,0,6);
		SelectiveAck sack;;
		sack.length = 4;
		sack.extension = 0;
		sack.bitmask = sack_data + 2;
		Ack(&sack,2);
		Ack(&sack,3);
		Ack(&sack,4);
		Header hdr;
		hdr.ack_nr = 0;
		hdr.wnd_size = 5000;
		retransmit_seq_nr.insert(1);
		wnd.packetReceived(&hdr,&sack,this);
		QVERIFY(wnd.numUnackedPackets() == 1);
		QVERIFY(update_rtt_called);
		QVERIFY(retransmit_ok);
	}
	
	void testMultipleRetransmits()
	{
		QByteArray pkt(200,0);
		
		RemoteWindow wnd;
		for (bt::Uint32 i = 0;i < 4;i++)
		{
			wnd.addPacket(pkt,i+1,bt::Now());
			QVERIFY(!wnd.allPacketsAcked());
			QVERIFY(wnd.numUnackedPackets() == i + 1);
		}
		
		// Selectively ack the last 3 packets
		bt::Uint8 sack_data[6];
		memset(sack_data,0,6);
		SelectiveAck sack;;
		sack.length = 4;
		sack.extension = 0;
		sack.bitmask = sack_data + 2;
		Ack(&sack,2);
		Ack(&sack,3);
		Ack(&sack,4);
		Header hdr;
		hdr.ack_nr = 0;
		hdr.wnd_size = 5000;
		retransmit_seq_nr.insert(1);
		wnd.packetReceived(&hdr,&sack,this);
		QVERIFY(wnd.numUnackedPackets() == 1);
		QVERIFY(update_rtt_called);
		QVERIFY(retransmit_ok);
	}
	
	void testMultipleRetransmits2()
	{
		QByteArray pkt(200,0);
		
		RemoteWindow wnd;
		for (bt::Uint32 i = 0;i < 10;i++)
		{
			wnd.addPacket(pkt,i+1,bt::Now());
			QVERIFY(!wnd.allPacketsAcked());
			QVERIFY(wnd.numUnackedPackets() == i + 1);
		}
		
		// Selectively ack the last 3 packets
		bt::Uint8 sack_data[6];
		memset(sack_data,0,6);
		SelectiveAck sack;;
		sack.length = 4;
		sack.extension = 0;
		sack.bitmask = sack_data + 2;
		Ack(&sack,8); 
		Ack(&sack,9);
		Ack(&sack,10); 
		Header hdr;
		hdr.ack_nr = 0;
		hdr.wnd_size = 5000;
		for (int i = 1;i < 8;i++)
			retransmit_seq_nr.insert(i);
		wnd.packetReceived(&hdr,&sack,this);
		QVERIFY(wnd.numUnackedPackets() == 7);
		QVERIFY(update_rtt_called);
		QVERIFY(retransmit_ok);
	}
	
	void testMultipleRetransmits3()
	{
		QByteArray pkt(200,0);
		
		RemoteWindow wnd;
		for (bt::Uint32 i = 0;i < 10;i++)
		{
			wnd.addPacket(pkt,i+1,bt::Now());
			QVERIFY(!wnd.allPacketsAcked());
			QVERIFY(wnd.numUnackedPackets() == i + 1);
		}
		
		// Selectively ack 3 random packets
		bt::Uint8 sack_data[6];
		memset(sack_data,0,6);
		SelectiveAck sack;;
		sack.length = 4;
		sack.extension = 0;
		sack.bitmask = sack_data + 2;
		Ack(&sack,3); 
		Ack(&sack,6);
		Ack(&sack,10); 
		Header hdr;
		hdr.ack_nr = 0;
		hdr.wnd_size = 5000;
		for (int i = 1;i <= 2;i++)
			retransmit_seq_nr.insert(i);
		wnd.packetReceived(&hdr,&sack,this);
		QVERIFY(wnd.numUnackedPackets() == 7);
		QVERIFY(update_rtt_called);
		QVERIFY(retransmit_ok);
	}
};

QTEST_MAIN(RemoteWindowTest)

#include "remotewindowtest.moc"