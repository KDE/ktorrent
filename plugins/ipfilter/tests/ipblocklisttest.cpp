/*
    SPDX-FileCopyrightText: 2012 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "../ipblocklist.h"
#include <QtTest>
#include <net/address.h>
#include <util/log.h>

class IPBlockListTest : public QObject
{
    Q_OBJECT
private:
private Q_SLOTS:
    void initTestCase()
    {
        bt::InitLog(QStringLiteral("ipblocklisttest.log"), false, true);
    }

    void cleanupTestCase()
    {
    }

    void testBlockList()
    {
        kt::IPBlockList bl;
        bl.addBlock(kt::IPBlock(QStringLiteral("1.0.0.0"), QStringLiteral("50.255.255.255")));
        bl.addBlock(kt::IPBlock(QStringLiteral("127.0.0.0"), QStringLiteral("127.255.255.255")));
        bl.addBlock(kt::IPBlock(QStringLiteral("129.0.0.0"), QStringLiteral("129.255.255.255")));
        bl.addBlock(kt::IPBlock(QStringLiteral("131.0.0.0"), QStringLiteral("137.255.255.255")));
        bl.addBlock(kt::IPBlock(QStringLiteral("140.0.0.0"), QStringLiteral("200.255.255.255")));

        QVERIFY(bl.blocked(net::Address(QStringLiteral("25.25.25.25"), 0)));
        QVERIFY(!bl.blocked(net::Address(QStringLiteral("75.25.25.25"), 0)));
        QVERIFY(!bl.blocked(net::Address(QStringLiteral("126.255.255.255"), 0)));
        QVERIFY(bl.blocked(net::Address(QStringLiteral("127.25.25.25"), 0)));
        QVERIFY(!bl.blocked(net::Address(QStringLiteral("130.255.255.255"), 0)));
        QVERIFY(bl.blocked(net::Address(QStringLiteral("135.25.25.25"), 0)));
        QVERIFY(!bl.blocked(net::Address(QStringLiteral("138.255.255.255"), 0)));
        QVERIFY(bl.blocked(net::Address(QStringLiteral("197.25.25.25"), 0)));
    }

private:
};

QTEST_MAIN(IPBlockListTest)

#include "ipblocklisttest.moc"
