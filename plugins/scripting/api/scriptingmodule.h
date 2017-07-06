/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
#ifndef KTSCRIPTINGMODULE_H
#define KTSCRIPTINGMODULE_H

#include <QObject>
#include <kross/core/object.h>
#include "scriptablegroup.h"

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
        ScriptingModule(GUIInterface* gui, CoreInterface* core, QObject* parent);
        ~ScriptingModule();

    public slots:
        /// Get the scripts directory
        QString scriptsDir() const;

        /// Get the data directory of a script
        QString scriptDir(const QString& script) const;

        /// Read a config entry
        QString readConfigEntry(const QString& group, const QString& name, const QString& default_value);
        int readConfigEntryInt(const QString& group, const QString& name, int default_value);
        float readConfigEntryFloat(const QString& group, const QString& name, float default_value);
        bool readConfigEntryBool(const QString& group, const QString& name, bool default_value);

        /// Write a config entry
        void writeConfigEntry(const QString& group, const QString& name, const QString& value);
        void writeConfigEntryInt(const QString& group, const QString& name, int value);
        void writeConfigEntryFloat(const QString& group, const QString& name, float value);
        void writeConfigEntryBool(const QString& group, const QString& name, bool value);

        /// Sync a group
        void syncConfig(const QString& group);

        /// Create a timer
        QObject* createTimer(bool single_shot);

        /// Add a new scriptable group
        bool addGroup(const QString& name, const QString& icon, const QString& path, Kross::Object::Ptr obj);

        /// Remove a previously added group
        void removeGroup(const QString& name);

    private:
        GUIInterface* gui;
        CoreInterface* core;
        QMap<QString, ScriptableGroup*> sgroups;
    };

}

#endif
