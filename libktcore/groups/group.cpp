/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
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

#include <torrent/queuemanager.h>
#include "group.h"

namespace kt
{
    Group::Policy::Policy()
    {
        max_share_ratio = max_seed_time = 0.0f;
        max_download_rate = max_upload_rate = 0;
        only_apply_on_new_torrents = false;
    }

    Group::Group(const QString& name, int flags, const QString& path) :
        name(name),
        flags(flags),
        path(path),
        running(0),
        total(0)
    {}


    Group::~Group()
    {}

    void Group::save(bt::BEncoder*)
    {
    }

    void Group::load(bt::BDictNode*)
    {
    }

    void Group::setIconByName(const QString& in)
    {
        icon_name = in;
        // KF5 icon = SmallIcon(in);
    }

    void Group::rename(const QString& nn)
    {
        name = nn;
    }

    void Group::torrentRemoved(TorrentInterface*)
    {}

    void Group::removeTorrent(TorrentInterface*)
    {}

    void Group::addTorrent(TorrentInterface* , bool)
    {}

    void Group::setGroupPolicy(const Policy& p)
    {
        policy = p;
        policyChanged();
    }

    void Group::policyChanged()
    {
    }

    void Group::updateCount(QueueManager* qman)
    {
        total = running = 0;
        for (QueueManager::iterator j = qman->begin(); j != qman->end(); j++)
        {
            if (isMember(*j))
            {
                total++;
                if ((*j)->getStats().running)
                    running++;
            }
        }
    }
}
