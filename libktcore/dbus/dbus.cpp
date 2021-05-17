/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QDBusConnection>
#include <QFile>
#include <QTimer>

#include <KConfig>

#include "dbus.h"
#include "dbusgroup.h"
#include "dbussettings.h"
#include "dbustorrent.h"
#include <groups/groupmanager.h>
#include <interfaces/coreinterface.h>
#include <interfaces/functions.h>
#include <interfaces/guiinterface.h>
#include <interfaces/torrentinterface.h>
#include <torrent/queuemanager.h>
#include <util/log.h>
#include <util/sha1hash.h>

using namespace bt;

namespace kt
{
DBus::DBus(GUIInterface *gui, CoreInterface *core, QObject *parent)
    : QObject(parent)
    , gui(gui)
    , core(core)
{
    torrent_map.setAutoDelete(true);
    group_map.setAutoDelete(true);

    QDBusConnection::sessionBus().registerObject(QLatin1String("/core"),
                                                 this,
                                                 QDBusConnection::ExportScriptableSlots | QDBusConnection::ExportScriptableSignals);

    connect(core, &CoreInterface::torrentAdded, this, qOverload<bt::TorrentInterface *>(&DBus::torrentAdded));
    connect(core, &CoreInterface::torrentRemoved, this, qOverload<bt::TorrentInterface *>(&DBus::torrentRemoved));
    connect(core, &CoreInterface::torrentStoppedByError, this, qOverload<bt::TorrentInterface *, QString>(&DBus::torrentStoppedByError));
    connect(core, &CoreInterface::finished, this, qOverload<bt::TorrentInterface *>(&DBus::finished));
    connect(core, &CoreInterface::settingsChanged, this, &DBus::settingsChanged);

    // fill the map with torrents
    const kt::QueueManager *const qman = core->getQueueManager();
    for (bt::TorrentInterface *i : *qman) {
        torrentAdded(i);
    }

    connect(qman, &kt::QueueManager::suspendStateChanged, this, &DBus::suspendStateChanged);

    kt::GroupManager *gman = core->getGroupManager();
    connect(gman, &kt::GroupManager::groupAdded, this, &DBus::groupAdded);
    connect(gman, &kt::GroupManager::groupRemoved, this, &DBus::groupRemoved);
    kt::GroupManager::Itr i = gman->begin();
    while (i != gman->end()) {
        if (i->second->groupFlags() & Group::CUSTOM_GROUP)
            groupAdded(i->second);
        i++;
    }

    dbus_settings = new DBusSettings(core, this);
}

DBus::~DBus()
{
}

QStringList DBus::torrents()
{
    QStringList tors;
    DBusTorrentItr i = torrent_map.begin();
    while (i != torrent_map.end()) {
        tors.append(i->first);
        i++;
    }

    return tors;
}

void DBus::start(const QString &info_hash)
{
    DBusTorrent *tc = torrent_map.find(info_hash);
    if (!tc)
        return;

    core->getQueueManager()->start(tc->torrent());
}

void DBus::stop(const QString &info_hash)
{
    DBusTorrent *tc = torrent_map.find(info_hash);
    if (!tc)
        return;

    core->getQueueManager()->stop(tc->torrent());
}

void DBus::startAll()
{
    core->startAll();
}

void DBus::stopAll()
{
    core->stopAll();
}

void DBus::torrentAdded(bt::TorrentInterface *tc)
{
    DBusTorrent *db = new DBusTorrent(tc, this);
    torrent_map.insert(db->infoHash(), db);
    torrentAdded(db->infoHash());
}

void DBus::torrentRemoved(bt::TorrentInterface *tc)
{
    DBusTorrent *db = torrent_map.find(tc->getInfoHash().toString());
    if (db) {
        QString ih = db->infoHash();
        torrentRemoved(ih);
        torrent_map.erase(ih);
    }
}

void DBus::finished(bt::TorrentInterface *tc)
{
    DBusTorrent *db = torrent_map.find(tc->getInfoHash().toString());
    if (db) {
        QString ih = db->infoHash();
        finished(ih);
    }
}

void DBus::torrentStoppedByError(bt::TorrentInterface *tc, QString msg)
{
    DBusTorrent *db = torrent_map.find(tc->getInfoHash().toString());
    if (db) {
        QString ih = db->infoHash();
        torrentStoppedByError(ih, msg);
    }
}

void DBus::load(const QString &url, const QString &group)
{
    core->load(QFile::exists(url) ? QUrl::fromLocalFile(url) : QUrl(url), group);
}

void DBus::loadSilently(const QString &url, const QString &group)
{
    core->loadSilently(QFile::exists(url) ? QUrl::fromLocalFile(url) : QUrl(url), group);
}

QStringList DBus::groups() const
{
    QStringList ret;
    kt::GroupManager *gman = core->getGroupManager();
    kt::GroupManager::Itr i = gman->begin();
    while (i != gman->end()) {
        if (i->second->groupFlags() & Group::CUSTOM_GROUP)
            ret << i->first;
        i++;
    }
    return ret;
}

bool DBus::addGroup(const QString &group)
{
    kt::GroupManager *gman = core->getGroupManager();
    return gman->newGroup(group) != 0;
}

bool DBus::removeGroup(const QString &group)
{
    kt::GroupManager *gman = core->getGroupManager();
    Group *g = gman->find(group);
    if (!g)
        return false;

    gman->removeGroup(g);
    return true;
}

void DBus::groupAdded(kt::Group *g)
{
    if (g->groupFlags() & Group::CUSTOM_GROUP)
        group_map.insert(g, new DBusGroup(g, core->getGroupManager(), this));
}

void DBus::groupRemoved(kt::Group *g)
{
    group_map.erase(g);
}

QObject *DBus::torrent(const QString &info_hash)
{
    return torrent_map.find(info_hash);
}

QObject *DBus::group(const QString &name)
{
    kt::GroupManager *gman = core->getGroupManager();
    kt::GroupManager::Itr i = gman->begin();
    while (i != gman->end()) {
        if (i->first == name)
            return group_map.find(i->second);
        i++;
    }
    return 0;
}

void DBus::log(const QString &line)
{
    Out(SYS_GEN | LOG_NOTICE) << line << endl;
}

void DBus::remove(const QString &info_hash, bool data_to)
{
    DBusTorrent *tc = torrent_map.find(info_hash);
    if (!tc)
        return;

    core->remove(tc->torrent(), data_to);
}

void DBus::removeDelayed(const QString &info_hash, bool data_to)
{
    delayed_removal_map.insert(info_hash, data_to);
    QTimer::singleShot(500, this, &DBus::delayedTorrentRemoval);
}

void DBus::delayedTorrentRemoval()
{
    for (QMap<QString, bool>::const_iterator i = delayed_removal_map.cbegin(); i != delayed_removal_map.cend(); i++)
        remove(i.key(), i.value());

    delayed_removal_map.clear();
}

void DBus::setSuspended(bool suspend)
{
    core->setSuspendedState(suspend);
}

bool DBus::suspended()
{
    return core->getSuspendedState();
}

uint DBus::numTorrentsRunning() const
{
    return core->getNumTorrentsRunning();
}

uint DBus::numTorrentsNotRunning() const
{
    return core->getNumTorrentsNotRunning();
}

QString DBus::dataDir() const
{
    return kt::DataDir();
}

void DBus::orderQueue()
{
    core->getQueueManager()->orderQueue();
}

void DBus::reindexQueue()
{
    core->getQueueManager()->reindexQueue();
}

QObject *DBus::settings()
{
    return dbus_settings;
}

}
