/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QStandardPaths>
#include <QTimer>

#include <KConfigGroup>
#include <KSharedConfig>

#include "scriptablegroup.h"
#include "scriptingmodule.h"
#include <groups/groupmanager.h>
#include <interfaces/coreinterface.h>
#include <util/functions.h>

namespace kt
{
ScriptingModule::ScriptingModule(GUIInterface *gui, CoreInterface *core, QObject *parent)
    : QObject(parent)
    , gui(gui)
    , core(core)
{
}

ScriptingModule::~ScriptingModule()
{
}

QString ScriptingModule::scriptsDir() const
{
    QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("ktorrent/scripts"), QStandardPaths::LocateDirectory);
    if (dirs.count() == 0)
        return QString();

    QString ret = dirs.front();
    if (!ret.endsWith(bt::DirSeparator()))
        ret += bt::DirSeparator();

    return ret;
}

QString ScriptingModule::scriptDir(const QString &script) const
{
    QStringList dirs =
        QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("ktorrent/scripts/") + script, QStandardPaths::LocateDirectory);
    if (dirs.count() == 0)
        return QString();

    QString ret = dirs.front();
    if (!ret.endsWith(bt::DirSeparator()))
        ret += bt::DirSeparator();

    return ret;
}

QString ScriptingModule::readConfigEntry(const QString &group, const QString &name, const QString &default_value)
{
    KConfigGroup g = KSharedConfig::openConfig()->group(group);
    return g.readEntry(name, default_value);
}

bool ScriptingModule::readConfigEntryBool(const QString &group, const QString &name, bool default_value)
{
    KConfigGroup g = KSharedConfig::openConfig()->group(group);
    return g.readEntry(name, default_value);
}

int ScriptingModule::readConfigEntryInt(const QString &group, const QString &name, int default_value)
{
    KConfigGroup g = KSharedConfig::openConfig()->group(group);
    return g.readEntry(name, default_value);
}

float ScriptingModule::readConfigEntryFloat(const QString &group, const QString &name, float default_value)
{
    KConfigGroup g = KSharedConfig::openConfig()->group(group);
    return g.readEntry(name, default_value);
}

void ScriptingModule::writeConfigEntry(const QString &group, const QString &name, const QString &value)
{
    KConfigGroup g = KSharedConfig::openConfig()->group(group);
    g.writeEntry(name, value);
}

void ScriptingModule::writeConfigEntryBool(const QString &group, const QString &name, bool value)
{
    KConfigGroup g = KSharedConfig::openConfig()->group(group);
    g.writeEntry(name, value);
}

void ScriptingModule::writeConfigEntryInt(const QString &group, const QString &name, int value)
{
    KConfigGroup g = KSharedConfig::openConfig()->group(group);
    g.writeEntry(name, value);
}

void ScriptingModule::writeConfigEntryFloat(const QString &group, const QString &name, float value)
{
    KConfigGroup g = KSharedConfig::openConfig()->group(group);
    g.writeEntry(name, value);
}

void ScriptingModule::syncConfig(const QString &group)
{
    KConfigGroup g = KSharedConfig::openConfig()->group(group);
    g.sync();
}

QObject *ScriptingModule::createTimer(bool single_shot)
{
    QTimer *t = new QTimer(this);
    t->setSingleShot(single_shot);
    return t;
}

bool ScriptingModule::addGroup(const QString &name, const QString &icon, const QString &path, Kross::Object::Ptr obj)
{
    ScriptableGroup *g = new ScriptableGroup(name, icon, path, obj, core->getExternalInterface());
    kt::GroupManager *gman = core->getGroupManager();
    gman->addDefaultGroup(g);
    sgroups.insert(name, g);
    return true;
}

void ScriptingModule::removeGroup(const QString &name)
{
    if (!sgroups.contains(name))
        return;

    kt::GroupManager *gman = core->getGroupManager();
    ScriptableGroup *g = sgroups[name];
    sgroups.remove(name);
    gman->removeDefaultGroup(g);
}
}
