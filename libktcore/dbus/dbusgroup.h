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

#ifndef KTDBUSGROUP_H
#define KTDBUSGROUP_H

#include <QObject>

namespace kt
{
    class Group;
    class GroupManager;

    /**
        @author
    */
    class DBusGroup : public QObject
    {
        Q_OBJECT
        Q_CLASSINFO("D-Bus Interface", "org.ktorrent.group")
    public:
        DBusGroup(Group* g, GroupManager* gman, QObject* parent);
        ~DBusGroup();

    public Q_SLOTS:
        Q_SCRIPTABLE QString name() const;
        Q_SCRIPTABLE QString icon() const;
        Q_SCRIPTABLE QString defaultSaveLocation() const;
        Q_SCRIPTABLE void setDefaultSaveLocation(const QString& dir);
        Q_SCRIPTABLE QString defaultMoveOnCompletionLocation() const;
        Q_SCRIPTABLE void setDefaultMoveOnCompletionLocation(const QString& dir);
        Q_SCRIPTABLE double maxShareRatio() const;
        Q_SCRIPTABLE void setMaxShareRatio(double ratio);
        Q_SCRIPTABLE double maxSeedTime() const;
        Q_SCRIPTABLE void setMaxSeedTime(double hours);
        Q_SCRIPTABLE uint maxUploadSpeed() const;
        Q_SCRIPTABLE void setMaxUploadSpeed(uint speed);
        Q_SCRIPTABLE uint maxDownloadSpeed() const;
        Q_SCRIPTABLE void setMaxDownloadSpeed(uint speed);
        Q_SCRIPTABLE bool onlyApplyOnNewTorrents() const;
        Q_SCRIPTABLE void setOnlyApplyOnNewTorrents(bool on);

    private:
        Group* group;
        GroupManager* gman;
    };

}

#endif
