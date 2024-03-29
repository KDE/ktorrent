/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QDBusConnection>

#include "dbusgroup.h"
#include <groups/group.h>
#include <groups/groupmanager.h>

namespace kt
{
static bool ValidCharacter(const QChar &c)
{
    ushort u = c.unicode();
    return (u >= 'a' && u <= 'z') || (u >= 'A' && u <= 'Z') || (u >= '0' && u <= '9') || (u == '_');
}

static bool ValidDBusName(const QString &s)
{
    if (s.isEmpty())
        return false;

    QString ret = s;
    if (ret[0].isDigit())
        return false;

    for (int i = 0; i < s.length(); i++) {
        QChar c = ret[i];
        if (!ValidCharacter(c))
            return false;
    }

    return true;
}

DBusGroup::DBusGroup(Group *g, GroupManager *gman, QObject *parent)
    : QObject(parent)
    , group(g)
    , gman(gman)
{
    QString name = g->groupName();
    if (!ValidDBusName(name)) {
        static int invalid_groups = 0;
        while (true) {
            name = QStringLiteral("group_") + QString::number(invalid_groups++);
            if (!gman->find(name))
                break;
        }
    }
    QString path = QStringLiteral("/group/") + name;
    QDBusConnection::sessionBus().registerObject(path, this, QDBusConnection::ExportScriptableSlots | QDBusConnection::ExportScriptableSignals);
}

DBusGroup::~DBusGroup()
{
}

QString DBusGroup::name() const
{
    return group->groupName();
}

QString DBusGroup::icon() const
{
    return group->groupIconName();
}

QString DBusGroup::defaultSaveLocation() const
{
    const Group::Policy &p = group->groupPolicy();
    return p.default_save_location;
}

void DBusGroup::setDefaultSaveLocation(const QString &dir)
{
    Group::Policy p = group->groupPolicy();
    p.default_save_location = dir;
    group->setGroupPolicy(p);
    gman->saveGroups();
}

QString DBusGroup::defaultMoveOnCompletionLocation() const
{
    const Group::Policy &p = group->groupPolicy();
    return p.default_move_on_completion_location;
}

void DBusGroup::setDefaultMoveOnCompletionLocation(const QString &dir)
{
    Group::Policy p = group->groupPolicy();
    p.default_move_on_completion_location = dir;
    group->setGroupPolicy(p);
    gman->saveGroups();
}

double DBusGroup::maxShareRatio() const
{
    const Group::Policy &p = group->groupPolicy();
    return p.max_share_ratio;
}

void DBusGroup::setMaxShareRatio(double ratio)
{
    Group::Policy p = group->groupPolicy();
    p.max_share_ratio = ratio;
    group->setGroupPolicy(p);
    gman->saveGroups();
}

double DBusGroup::maxSeedTime() const
{
    const Group::Policy &p = group->groupPolicy();
    return p.max_seed_time;
}

void DBusGroup::setMaxSeedTime(double hours)
{
    Group::Policy p = group->groupPolicy();
    p.max_seed_time = hours;
    group->setGroupPolicy(p);
    gman->saveGroups();
}

uint DBusGroup::maxUploadSpeed() const
{
    const Group::Policy &p = group->groupPolicy();
    return p.max_upload_rate;
}

void DBusGroup::setMaxUploadSpeed(uint speed)
{
    Group::Policy p = group->groupPolicy();
    p.max_upload_rate = speed;
    group->setGroupPolicy(p);
    gman->saveGroups();
}

uint DBusGroup::maxDownloadSpeed() const
{
    const Group::Policy &p = group->groupPolicy();
    return p.max_download_rate;
}

void DBusGroup::setMaxDownloadSpeed(uint speed)
{
    Group::Policy p = group->groupPolicy();
    p.max_download_rate = speed;
    group->setGroupPolicy(p);
    gman->saveGroups();
}

bool DBusGroup::onlyApplyOnNewTorrents() const
{
    const Group::Policy &p = group->groupPolicy();
    return p.only_apply_on_new_torrents;
}

void DBusGroup::setOnlyApplyOnNewTorrents(bool on)
{
    Group::Policy p = group->groupPolicy();
    p.only_apply_on_new_torrents = on;
    group->setGroupPolicy(p);
    gman->saveGroups();
}
}

#include "moc_dbusgroup.cpp"
