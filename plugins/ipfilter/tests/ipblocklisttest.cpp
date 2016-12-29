/***************************************************************************
 *   Copyright (C) 2012 by                                                 *
 *   Joris Guisson <joris.guisson@gmail.com>                               *
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
#include <util/log.h>
#include <net/address.h>
// [Fonic]
//#include <plugins/ipfilter/ipblocklist.h>
#include "../ipblocklist.h"

class IPBlockListTest : public QObject
{
    Q_OBJECT
private:

private slots:
    void initTestCase()
    {
        bt::InitLog("ipblocklisttest.log", false, true);
    }

    void cleanupTestCase()
    {
    }


    void testBlockList()
    {
        kt::IPBlockList bl;
        bl.addBlock(kt::IPBlock("1.0.0.0", "50.255.255.255"));
        bl.addBlock(kt::IPBlock("127.0.0.0", "127.255.255.255"));
        bl.addBlock(kt::IPBlock("129.0.0.0", "129.255.255.255"));
        bl.addBlock(kt::IPBlock("131.0.0.0", "137.255.255.255"));
        bl.addBlock(kt::IPBlock("140.0.0.0", "200.255.255.255"));

        QVERIFY(bl.blocked(net::Address("25.25.25.25", 0)));
        QVERIFY(!bl.blocked(net::Address("75.25.25.25", 0)));
        QVERIFY(!bl.blocked(net::Address("126.255.255.255", 0)));
        QVERIFY(bl.blocked(net::Address("127.25.25.25", 0)));
        QVERIFY(!bl.blocked(net::Address("130.255.255.255", 0)));
        QVERIFY(bl.blocked(net::Address("135.25.25.25", 0)));
        QVERIFY(!bl.blocked(net::Address("138.255.255.255", 0)));
        QVERIFY(bl.blocked(net::Address("197.25.25.25", 0)));
    }



private:

};

QTEST_MAIN(IPBlockListTest)

#include "ipblocklisttest.moc"
