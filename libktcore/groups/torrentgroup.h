/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTTORRENTGROUP_H
#define KTTORRENTGROUP_H

#include <groups/group.h>
#include <set>
#include <util/sha1hash.h>

namespace kt
{
class QueueManager;

/**
    @author Joris Guisson <joris.guisson@gmail.com>
*/
class KTCORE_EXPORT TorrentGroup : public Group
{
    Q_OBJECT
public:
    TorrentGroup(const QString &name);
    ~TorrentGroup() override;

    bool isMember(TorrentInterface *tor) override;
    void save(bt::BEncoder *enc) override;
    void load(bt::BDictNode *n) override;
    void torrentRemoved(TorrentInterface *tor) override;
    void removeTorrent(TorrentInterface *tor) override;
    void addTorrent(TorrentInterface *tor, bool new_torrent) override;
    void policyChanged() override;

    void add(TorrentInterface *tor);
    void remove(TorrentInterface *tor);
    void loadTorrents(QueueManager *qman);

Q_SIGNALS:
    /// Emitted when a torrent has been added
    void torrentAdded(Group *g);

    /// Emitted when a torrent has been removed
    void torrentRemoved(Group *g);

private:
    std::set<TorrentInterface *> torrents;
    std::set<bt::SHA1Hash> hashes;
};

}

#endif
