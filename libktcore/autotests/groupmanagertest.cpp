/*
   SPDX-FileCopyrightText: 2023 (c) Jack Hill <jackhill3103@gmail.com>
   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <groups/groupmanager.h>
#include <groups/torrentgroup.h>

#include <QtTest>

namespace kt
{

class GroupManagerTests : public QObject
{
    Q_OBJECT

private:
    const QString customGroupPath = QStringLiteral("/all/custom/");
    const QString customGroup1Str = QStringLiteral("customGroup1");
    const QString customGroup2Str = QStringLiteral("customGroup2");
    const QString customGroup3Str = QStringLiteral("customGroup3");

private Q_SLOTS:

    void addUniqueCustomGroups()
    {
        GroupManager groupManager;
        QStringList expectedGroupNames;

        groupManager.newGroup(customGroup1Str);
        expectedGroupNames = QStringList{customGroup1Str};
        auto *g1 = groupManager.find(customGroup1Str);

        QVERIFY(g1 != nullptr);
        QVERIFY(groupManager.canRemove(g1));
        QCOMPARE(groupManager.findByPath(customGroupPath + customGroup1Str), g1);
        QCOMPARE(groupManager.customGroupNames(), expectedGroupNames);

        groupManager.newGroup(customGroup2Str);
        expectedGroupNames = QStringList{customGroup1Str, customGroup2Str};
        auto *g2 = groupManager.find(customGroup2Str);

        QVERIFY(g2 != nullptr);
        QVERIFY(groupManager.canRemove(g2));
        QCOMPARE(groupManager.findByPath(customGroupPath + customGroup2Str), g2);
        QCOMPARE(groupManager.customGroupNames(), expectedGroupNames);
        QVERIFY(g1 != g2);
    }

    void addCustomGroupTwice()
    {
        GroupManager groupManager;
        QStringList expectedGroupNames;

        groupManager.newGroup(customGroup1Str);
        expectedGroupNames = QStringList{customGroup1Str};
        auto *g1 = groupManager.find(customGroup1Str);

        QVERIFY(g1 != nullptr);
        QVERIFY(groupManager.canRemove(g1));
        QCOMPARE(groupManager.findByPath(customGroupPath + customGroup1Str), g1);
        QCOMPARE(groupManager.customGroupNames(), expectedGroupNames);

        groupManager.newGroup(customGroup1Str);
        expectedGroupNames = QStringList{customGroup1Str};
        auto *g1_new = groupManager.find(customGroup1Str);

        QCOMPARE(g1, g1_new);
        QCOMPARE(groupManager.findByPath(customGroupPath + customGroup1Str), g1);
        QCOMPARE(groupManager.customGroupNames(), expectedGroupNames);
    }

    void removeCustomGroup()
    {
        GroupManager groupManager;
        QStringList expectedGroupNames;

        groupManager.newGroup(customGroup1Str);
        expectedGroupNames = QStringList{customGroup1Str};
        auto *g1 = groupManager.find(customGroup1Str);

        QVERIFY(g1 != nullptr);
        QVERIFY(groupManager.canRemove(g1));
        QCOMPARE(groupManager.findByPath(customGroupPath + customGroup1Str), g1);
        QCOMPARE(groupManager.customGroupNames(), expectedGroupNames);

        groupManager.removeGroup(g1);
        expectedGroupNames = QStringList{};
        g1 = groupManager.find(customGroup1Str);

        QCOMPARE(g1, nullptr);
        QVERIFY(!groupManager.canRemove(g1));
        QCOMPARE(groupManager.findByPath(customGroupPath + customGroup1Str), g1);
        QCOMPARE(groupManager.customGroupNames(), expectedGroupNames);
    }

    void removeInvalidCustomGroup()
    {
        GroupManager groupManager;
        QStringList expectedGroupNames;

        groupManager.newGroup(customGroup1Str);
        expectedGroupNames = QStringList{customGroup1Str};
        auto *g1 = groupManager.find(customGroup1Str);

        QVERIFY(g1 != nullptr);
        QVERIFY(groupManager.canRemove(g1));
        QCOMPARE(groupManager.findByPath(customGroupPath + customGroup1Str), g1);
        QCOMPARE(groupManager.customGroupNames(), expectedGroupNames);

        TorrentGroup *tg2 = nullptr;
        QVERIFY(!groupManager.canRemove(tg2));

        groupManager.removeGroup(tg2);
        auto *g2 = groupManager.find(customGroup2Str);
        expectedGroupNames = QStringList{customGroup1Str};

        QCOMPARE(g2, nullptr);
        QCOMPARE(groupManager.customGroupNames(), expectedGroupNames);

        auto *tg3 = new TorrentGroup{customGroup3Str};
        QVERIFY(groupManager.canRemove(tg3));

        groupManager.removeGroup(tg3);
        auto *g3 = groupManager.find(customGroup3Str);
        expectedGroupNames = QStringList{customGroup1Str};

        QCOMPARE(g3, nullptr);
        QCOMPARE(groupManager.customGroupNames(), expectedGroupNames);
    }

    void renameCustomGroup()
    {
        GroupManager groupManager;
        QStringList expectedGroupNames;

        groupManager.newGroup(customGroup1Str);
        expectedGroupNames = QStringList{customGroup1Str};
        auto *g1 = groupManager.find(customGroup1Str);

        QVERIFY(g1 != nullptr);
        QVERIFY(groupManager.canRemove(g1));
        QCOMPARE(groupManager.findByPath(customGroupPath + customGroup1Str), g1);
        QCOMPARE(groupManager.customGroupNames(), expectedGroupNames);

        groupManager.renameGroup(customGroup1Str, customGroup2Str);
        g1 = groupManager.find(customGroup1Str);
        auto *g2 = groupManager.find(customGroup2Str);
        expectedGroupNames = QStringList{customGroup2Str};

        QVERIFY(g1 == nullptr);
        QVERIFY(g2 != nullptr);
        QVERIFY(!groupManager.canRemove(g1));
        QVERIFY(groupManager.canRemove(g2));
        QCOMPARE(groupManager.findByPath(customGroupPath + customGroup1Str), g2);
        QCOMPARE(groupManager.customGroupNames(), expectedGroupNames);
    }

    void renameInvalidCustomGroup()
    {
        GroupManager groupManager;
        QStringList expectedGroupNames;

        groupManager.newGroup(customGroup1Str);
        expectedGroupNames = QStringList{customGroup1Str};
        auto *g1 = groupManager.find(customGroup1Str);

        QVERIFY(g1 != nullptr);
        QVERIFY(groupManager.canRemove(g1));
        QCOMPARE(groupManager.findByPath(customGroupPath + customGroup1Str), g1);
        QCOMPARE(groupManager.customGroupNames(), expectedGroupNames);

        groupManager.renameGroup(customGroup2Str, customGroup3Str);
        g1 = groupManager.find(customGroup1Str);
        auto *g2 = groupManager.find(customGroup2Str);
        expectedGroupNames = QStringList{customGroup1Str};

        QVERIFY(g1 != nullptr);
        QVERIFY(groupManager.canRemove(g1));
        QCOMPARE(groupManager.findByPath(customGroupPath + customGroup1Str), g1);
        QCOMPARE(groupManager.customGroupNames(), expectedGroupNames);
        QVERIFY(g2 == nullptr);
        QVERIFY(!groupManager.canRemove(g2));
        QCOMPARE(groupManager.findByPath(customGroupPath + customGroup2Str), g2);
        QCOMPARE(groupManager.customGroupNames(), expectedGroupNames);
    }
};
}

QTEST_GUILESS_MAIN(kt::GroupManagerTests)

#include "groupmanagertest.moc"
