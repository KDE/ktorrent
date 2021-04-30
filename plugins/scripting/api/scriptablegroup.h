/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTAPISCRIPTABLEGROUP_H
#define KTAPISCRIPTABLEGROUP_H

#include <Kross/Core/Object>
#include <groups/group.h>

namespace kt
{
class DBus;

/**
    Group which uses objects in a script to determine if a torrent is a member of the group.
*/
class ScriptableGroup : public kt::Group
{
    Kross::Object::Ptr script;
    DBus *api;

public:
    ScriptableGroup(const QString &name, const QString &icon, const QString &path, Kross::Object::Ptr script, DBus *api);
    ~ScriptableGroup() override;

    bool isMember(bt::TorrentInterface *tor) override;
};

}

#endif
