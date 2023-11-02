/*
   SPDX-FileCopyrightText: 2023 (c) Jack Hill <jackhill3103@gmail.com>
   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QAbstractItemModelTester>
#include <groups/groupmanager.h>
#include <groups/grouptreemodel.h>
#include <groups/torrentgroup.h>

#include <QtTest>

namespace kt
{

class GroupTreeModelTests : public QObject
{
    Q_OBJECT

private:
    const QString customGroup1Str = QStringLiteral("customGroup1");
    const QString customGroup2Str = QStringLiteral("customGroup2");
    const QString customGroup3Str = QStringLiteral("customGroup3");

private Q_SLOTS:

    void addUniqueCustomGroups()
    {
        GroupManager groupManager;
        GroupTreeModel groupModel{&groupManager, this};
        QAbstractItemModelTester testModel(&groupModel);

        QSignalSpy beginInsertRowsSpy(&groupModel, &GroupTreeModel::rowsAboutToBeInserted);
        QSignalSpy endInsertRowsSpy(&groupModel, &GroupTreeModel::rowsInserted);
        QSignalSpy beginRemoveRowsSpy(&groupModel, &GroupTreeModel::rowsAboutToBeRemoved);
        QSignalSpy endRemoveRowsSpy(&groupModel, &GroupTreeModel::rowsRemoved);
        QSignalSpy dataChangedSpy(&groupModel, &GroupTreeModel::dataChanged);

        QCOMPARE(beginInsertRowsSpy.count(), 0);
        QCOMPARE(endInsertRowsSpy.count(), 0);
        QCOMPARE(beginRemoveRowsSpy.count(), 0);
        QCOMPARE(endRemoveRowsSpy.count(), 0);
        QCOMPARE(dataChangedSpy.count(), 0);

        groupManager.newGroup(customGroup1Str);

        QCOMPARE(beginInsertRowsSpy.count(), 1);
        QCOMPARE(endInsertRowsSpy.count(), 1);
        QCOMPARE(beginRemoveRowsSpy.count(), 0);
        QCOMPARE(endRemoveRowsSpy.count(), 0);
        QCOMPARE(dataChangedSpy.count(), 0);

        groupManager.newGroup(customGroup2Str);

        QCOMPARE(beginInsertRowsSpy.count(), 2);
        QCOMPARE(endInsertRowsSpy.count(), 2);
        QCOMPARE(beginRemoveRowsSpy.count(), 0);
        QCOMPARE(endRemoveRowsSpy.count(), 0);
        QCOMPARE(dataChangedSpy.count(), 0);
    }

    void addCustomGroupTwice()
    {
        GroupManager groupManager;
        GroupTreeModel groupModel{&groupManager, this};
        QAbstractItemModelTester testModel(&groupModel);

        QSignalSpy beginInsertRowsSpy(&groupModel, &GroupTreeModel::rowsAboutToBeInserted);
        QSignalSpy endInsertRowsSpy(&groupModel, &GroupTreeModel::rowsInserted);
        QSignalSpy beginRemoveRowsSpy(&groupModel, &GroupTreeModel::rowsAboutToBeRemoved);
        QSignalSpy endRemoveRowsSpy(&groupModel, &GroupTreeModel::rowsRemoved);
        QSignalSpy dataChangedSpy(&groupModel, &GroupTreeModel::dataChanged);

        QCOMPARE(beginInsertRowsSpy.count(), 0);
        QCOMPARE(endInsertRowsSpy.count(), 0);
        QCOMPARE(beginRemoveRowsSpy.count(), 0);
        QCOMPARE(endRemoveRowsSpy.count(), 0);
        QCOMPARE(dataChangedSpy.count(), 0);

        groupManager.newGroup(customGroup1Str);

        QCOMPARE(beginInsertRowsSpy.count(), 1);
        QCOMPARE(endInsertRowsSpy.count(), 1);
        QCOMPARE(beginRemoveRowsSpy.count(), 0);
        QCOMPARE(endRemoveRowsSpy.count(), 0);
        QCOMPARE(dataChangedSpy.count(), 0);

        groupManager.newGroup(customGroup1Str);

        QCOMPARE(beginInsertRowsSpy.count(), 1);
        QCOMPARE(endInsertRowsSpy.count(), 1);
        QCOMPARE(beginRemoveRowsSpy.count(), 0);
        QCOMPARE(endRemoveRowsSpy.count(), 0);
        QCOMPARE(dataChangedSpy.count(), 0);
    }

    void removeCustomGroup()
    {
        GroupManager groupManager;
        GroupTreeModel groupModel{&groupManager, this};
        QAbstractItemModelTester testModel(&groupModel);

        QSignalSpy beginInsertRowsSpy(&groupModel, &GroupTreeModel::rowsAboutToBeInserted);
        QSignalSpy endInsertRowsSpy(&groupModel, &GroupTreeModel::rowsInserted);
        QSignalSpy beginRemoveRowsSpy(&groupModel, &GroupTreeModel::rowsAboutToBeRemoved);
        QSignalSpy endRemoveRowsSpy(&groupModel, &GroupTreeModel::rowsRemoved);
        QSignalSpy dataChangedSpy(&groupModel, &GroupTreeModel::dataChanged);

        QCOMPARE(beginInsertRowsSpy.count(), 0);
        QCOMPARE(endInsertRowsSpy.count(), 0);
        QCOMPARE(beginRemoveRowsSpy.count(), 0);
        QCOMPARE(endRemoveRowsSpy.count(), 0);
        QCOMPARE(dataChangedSpy.count(), 0);

        groupManager.newGroup(customGroup1Str);
        groupManager.newGroup(customGroup2Str);
        groupManager.newGroup(customGroup3Str);

        QCOMPARE(beginInsertRowsSpy.count(), 3);
        QCOMPARE(endInsertRowsSpy.count(), 3);
        QCOMPARE(beginRemoveRowsSpy.count(), 0);
        QCOMPARE(endRemoveRowsSpy.count(), 0);
        QCOMPARE(dataChangedSpy.count(), 0);

        groupManager.removeGroup(groupManager.find(customGroup1Str));

        QCOMPARE(beginInsertRowsSpy.count(), 3);
        QCOMPARE(endInsertRowsSpy.count(), 3);
        QCOMPARE(beginRemoveRowsSpy.count(), 1);
        QCOMPARE(endRemoveRowsSpy.count(), 1);
        QCOMPARE(dataChangedSpy.count(), 0);

        groupManager.removeGroup(groupManager.find(customGroup2Str));

        QCOMPARE(beginInsertRowsSpy.count(), 3);
        QCOMPARE(endInsertRowsSpy.count(), 3);
        QCOMPARE(beginRemoveRowsSpy.count(), 2);
        QCOMPARE(endRemoveRowsSpy.count(), 2);
        QCOMPARE(dataChangedSpy.count(), 0);

        groupManager.removeGroup(groupManager.find(customGroup3Str));

        QCOMPARE(beginInsertRowsSpy.count(), 3);
        QCOMPARE(endInsertRowsSpy.count(), 3);
        QCOMPARE(beginRemoveRowsSpy.count(), 3);
        QCOMPARE(endRemoveRowsSpy.count(), 3);
        QCOMPARE(dataChangedSpy.count(), 0);
    }

    void removeInvalidCustomGroup()
    {
        GroupManager groupManager;
        GroupTreeModel groupModel{&groupManager, this};
        QAbstractItemModelTester testModel(&groupModel);

        QSignalSpy beginInsertRowsSpy(&groupModel, &GroupTreeModel::rowsAboutToBeInserted);
        QSignalSpy endInsertRowsSpy(&groupModel, &GroupTreeModel::rowsInserted);
        QSignalSpy beginRemoveRowsSpy(&groupModel, &GroupTreeModel::rowsAboutToBeRemoved);
        QSignalSpy endRemoveRowsSpy(&groupModel, &GroupTreeModel::rowsRemoved);
        QSignalSpy dataChangedSpy(&groupModel, &GroupTreeModel::dataChanged);

        QCOMPARE(beginInsertRowsSpy.count(), 0);
        QCOMPARE(endInsertRowsSpy.count(), 0);
        QCOMPARE(beginRemoveRowsSpy.count(), 0);
        QCOMPARE(endRemoveRowsSpy.count(), 0);
        QCOMPARE(dataChangedSpy.count(), 0);

        groupManager.newGroup(customGroup1Str);

        QCOMPARE(beginInsertRowsSpy.count(), 1);
        QCOMPARE(endInsertRowsSpy.count(), 1);
        QCOMPARE(beginRemoveRowsSpy.count(), 0);
        QCOMPARE(endRemoveRowsSpy.count(), 0);
        QCOMPARE(dataChangedSpy.count(), 0);

        groupManager.removeGroup(groupManager.find(customGroup2Str));

        QCOMPARE(beginInsertRowsSpy.count(), 1);
        QCOMPARE(endInsertRowsSpy.count(), 1);
        QCOMPARE(beginRemoveRowsSpy.count(), 0);
        QCOMPARE(endRemoveRowsSpy.count(), 0);
        QCOMPARE(dataChangedSpy.count(), 0);

        groupManager.removeGroup(new TorrentGroup{customGroup3Str});

        QCOMPARE(beginInsertRowsSpy.count(), 1);
        QCOMPARE(endInsertRowsSpy.count(), 1);
        QCOMPARE(beginRemoveRowsSpy.count(), 0);
        QCOMPARE(endRemoveRowsSpy.count(), 0);
        QCOMPARE(dataChangedSpy.count(), 0);
    }
};
}

QTEST_GUILESS_MAIN(kt::GroupTreeModelTests)

#include "grouptreemodeltest.moc"
