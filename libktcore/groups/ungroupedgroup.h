/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTUNGROUPEDGROUP_H
#define KTUNGROUPEDGROUP_H

#include <groups/group.h>

namespace kt
{
class GroupManager;

/**
    @author
*/
class UngroupedGroup : public Group
{
public:
    UngroupedGroup(GroupManager *gman);
    ~UngroupedGroup() override;

    bool isMember(TorrentInterface *tor) override;

private:
    GroupManager *gman;
};

}

#endif
