/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef KTSCRIPTINGMODULE_H
#define KTSCRIPTINGMODULE_H

#include "scriptablegroup.h"
#include <QObject>
#include <kross/core/object.h>

namespace kt
{
class GUIInterface;
class CoreInterface;

/**
    Additional functions to be used in scripts
*/
class ScriptingModule : public QObject
{
    Q_OBJECT
public:
    ScriptingModule(GUIInterface *gui, CoreInterface *core, QObject *parent);
    ~ScriptingModule() override;

public Q_SLOTS:
    /// Get the scripts directory
    QString scriptsDir() const;

    /// Get the data directory of a script
    QString scriptDir(const QString &script) const;

    /// Read a config entry
    QString readConfigEntry(const QString &group, const QString &name, const QString &default_value);
    int readConfigEntryInt(const QString &group, const QString &name, int default_value);
    float readConfigEntryFloat(const QString &group, const QString &name, float default_value);
    bool readConfigEntryBool(const QString &group, const QString &name, bool default_value);

    /// Write a config entry
    void writeConfigEntry(const QString &group, const QString &name, const QString &value);
    void writeConfigEntryInt(const QString &group, const QString &name, int value);
    void writeConfigEntryFloat(const QString &group, const QString &name, float value);
    void writeConfigEntryBool(const QString &group, const QString &name, bool value);

    /// Sync a group
    void syncConfig(const QString &group);

    /// Create a timer
    QObject *createTimer(bool single_shot);

    /// Add a new scriptable group
    bool addGroup(const QString &name, const QString &icon, const QString &path, Kross::Object::Ptr obj);

    /// Remove a previously added group
    void removeGroup(const QString &name);

private:
    GUIInterface *gui;
    CoreInterface *core;
    QMap<QString, ScriptableGroup *> sgroups;
};

}

#endif
