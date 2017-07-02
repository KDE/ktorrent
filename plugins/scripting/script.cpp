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

#include <QMimeDatabase>
#include <QMimeType>

#include <KConfigGroup>
#include <KDesktopFile>
#include <Kross/Core/Manager>
#include <Kross/Core/ActionCollection>

#include <util/fileops.h>
#include <util/log.h>
#include "script.h"

using namespace bt;

namespace kt
{
    Script::Script(QObject* parent) : QObject(parent), action(nullptr), executing(false), can_be_removed(true)
    {
    }

    Script::Script(const QString& file, QObject* parent) : QObject(parent), file(file), action(nullptr), executing(false), can_be_removed(true)
    {
    }


    Script::~Script()
    {
        stop();
    }

    bool Script::loadFromDesktopFile(const QString& dir, const QString& desktop_file)
    {
        KDesktopFile df(dir + desktop_file);
        // check if everything is OK
        if (df.readType().trimmed() != QStringLiteral("KTorrentScript"))
            return false;

        info.name = df.readName();
        info.comment = df.readComment();
        info.icon = df.readIcon();

        KConfigGroup g = df.group("Desktop Entry");
        info.author = g.readEntry("X-KTorrent-Script-Author", QString());
        info.email = g.readEntry("X-KTorrent-Script-Email", QString());
        info.website = g.readEntry("X-KTorrent-Script-Website", QString());
        info.license = g.readEntry("X-KTorrent-Script-License", QString());
        file = g.readEntry("X-KTorrent-Script-File", QString());
        if (file.isEmpty() || !bt::Exists(dir + file)) // the script file must exist
            return false;

        file = dir + file;
        return true;
    }

    bool Script::executeable() const
    {
        return bt::Exists(file) && !Kross::Manager::self().interpreternameForFile(file).isNull();
    }

    bool Script::execute()
    {
        if (!bt::Exists(file) || action)
            return false;

        QMimeDatabase db;
        QMimeType mt = db.mimeTypeForFile(file);
        QString name = QFileInfo(file).fileName();
        action = new Kross::Action(this, name);
        action->setText(name);
        action->setDescription(name);
        action->setFile(file);
        action->setIconName(mt.iconName());
        QString interpreter = Kross::Manager::self().interpreternameForFile(file);
        if (interpreter.isNull())
        {
            delete action;
            action = nullptr;
            return false;
        }
        else
        {
            action->setInterpreter(interpreter);
            Kross::Manager::self().actionCollection()->addAction(file, action);
            action->trigger();
            executing = true;
            return true;
        }
    }

    void Script::stop()
    {
        if (!executing)
            return;

        // Call unload function if the script has one
        if (action->functionNames().contains(QStringLiteral("unload")))
        {
            QVariantList args;
            action->callFunction(QStringLiteral("unload"), args);
        }

        Kross::ActionCollection* col = Kross::Manager::self().actionCollection();
        col->removeAction(action->file());
        action->deleteLater();
        action = nullptr;
        executing = false;
    }

    QString Script::name() const
    {
        if (!info.name.isEmpty())
            return info.name;
        else if (action)
            return action->name();
        else
            return QFileInfo(file).fileName();
    }

    QString Script::iconName() const
    {
        QMimeDatabase db;
        if (!info.icon.isEmpty())
            return info.icon;
        else if (action)
            return action->iconName();
        else
            return db.mimeTypeForFile(file).iconName();
    }

    bool Script::hasConfigure() const
    {
        if (!action)
            return false;

        QStringList functions = action->functionNames();
        return functions.contains(QStringLiteral("configure"));
    }

    void Script::configure()
    {
        if (!action)
            return;

        QVariantList args;
        action->callFunction(QStringLiteral("configure"), args);
    }

}
