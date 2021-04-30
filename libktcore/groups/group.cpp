/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "group.h"
#include <torrent/queuemanager.h>

namespace kt
{
Group::Policy::Policy()
{
    max_share_ratio = max_seed_time = 0.0f;
    max_download_rate = max_upload_rate = 0;
    only_apply_on_new_torrents = false;
}

Group::Group(const QString &name, int flags, const QString &path)
    : name(name)
    , flags(flags)
    , path(path)
    , running(0)
    , total(0)
{
}

Group::~Group()
{
}

void Group::save(bt::BEncoder *)
{
}

void Group::load(bt::BDictNode *)
{
}

void Group::setIconByName(const QString &in)
{
    icon_name = in;
    // KF5 icon = SmallIcon(in);
}

void Group::rename(const QString &nn)
{
    name = nn;
}

void Group::torrentRemoved(TorrentInterface *)
{
}

void Group::removeTorrent(TorrentInterface *)
{
}

void Group::addTorrent(TorrentInterface *, bool)
{
}

void Group::setGroupPolicy(const Policy &p)
{
    policy = p;
    policyChanged();
}

void Group::policyChanged()
{
}

void Group::updateCount(QueueManager *qman)
{
    total = running = 0;
    for (bt::TorrentInterface *tor : qAsConst(*qman)) {
        if (isMember(tor)) {
            total++;
            if (tor->getStats().running)
                running++;
        }
    }
}
}
