/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ungroupedgroup.h"
#include "groupmanager.h"

#include <KLocalizedString>

namespace kt
{
UngroupedGroup::UngroupedGroup(GroupManager *gman)
    : Group(i18n("Ungrouped Torrents"), MIXED_GROUP, QStringLiteral("/all/ungrouped"))
    , gman(gman)
{
    setIconByName(QStringLiteral("application-x-bittorrent"));
}

UngroupedGroup::~UngroupedGroup()
{
}

bool UngroupedGroup::isMember(TorrentInterface *tor)
{
    for (GroupManager::CItr i = gman->begin(); i != gman->end(); i++)
        if ((i->second->groupFlags() & Group::CUSTOM_GROUP) && i->second->isMember(tor))
            return false;

    return true;
}

}
