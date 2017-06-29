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

#include "ungroupedgroup.h"
#include "groupmanager.h"

#include <KLocalizedString>

namespace kt
{

    UngroupedGroup::UngroupedGroup(GroupManager* gman) : Group(i18n("Ungrouped Torrents"), MIXED_GROUP, QStringLiteral("/all/ungrouped")), gman(gman)
    {
        setIconByName(QStringLiteral("application-x-bittorrent"));
    }


    UngroupedGroup::~UngroupedGroup()
    {
    }


    bool UngroupedGroup::isMember(TorrentInterface* tor)
    {
        for (GroupManager::Itr i = gman->begin(); i != gman->end(); i++)
            if ((i->second->groupFlags() & Group::CUSTOM_GROUP) && i->second->isMember(tor))
                return false;

        return true;
    }

}
