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

#include <util/log.h>
#include <util/error.h>
#include <util/sha1hash.h>
#include <bcodec/bencoder.h>
#include <bcodec/bnode.h>
#include <interfaces/torrentinterface.h>
#include "torrentgroup.h"
#include <torrent/queuemanager.h>
#include <util/fileops.h>

using namespace bt;

namespace kt
{

    TorrentGroup::TorrentGroup(const QString& name): Group(name, MIXED_GROUP | CUSTOM_GROUP, QLatin1String("/all/custom/") + name)
    {
        setIconByName(QStringLiteral("application-x-bittorrent"));
    }


    TorrentGroup::~TorrentGroup()
    {}


    bool TorrentGroup::isMember(TorrentInterface* tor)
    {
        return torrents.count(tor) > 0;
    }

    void TorrentGroup::add(TorrentInterface* tor)
    {
        torrents.insert(tor);
    }

    void TorrentGroup::remove(TorrentInterface* tor)
    {
        torrents.erase(tor);
    }

    void TorrentGroup::save(bt::BEncoder* enc)
    {
        enc->beginDict();
        enc->write(QByteArrayLiteral("name")); enc->write(name.toLocal8Bit());
        enc->write(QByteArrayLiteral("icon")); enc->write(icon_name.toLocal8Bit());
        enc->write(QByteArrayLiteral("hashes")); enc->beginList();
        std::set<TorrentInterface*>::iterator i = torrents.begin();
        while (i != torrents.end())
        {
            TorrentInterface* tc = *i;
            // write the info hash, because that will be unique for each torrent
            const bt::SHA1Hash& h = tc->getInfoHash();
            enc->write(h.getData(), 20);
            i++;
        }
        std::set<bt::SHA1Hash>::iterator j = hashes.begin();
        while (j != hashes.end())
        {
            enc->write(j->getData(), 20);
            j++;
        }
        enc->end();
        enc->write(QByteArrayLiteral("policy")); enc->beginDict();
        enc->write(QByteArrayLiteral("default_save_location")); enc->write(policy.default_save_location.toUtf8());
        enc->write(QByteArrayLiteral("max_share_ratio")); enc->write(QByteArray::number(policy.max_share_ratio));
        enc->write(QByteArrayLiteral("max_seed_time")); enc->write(QByteArray::number(policy.max_seed_time));
        enc->write(QByteArrayLiteral("max_upload_rate")); enc->write(policy.max_upload_rate);
        enc->write(QByteArrayLiteral("max_download_rate")); enc->write(policy.max_download_rate);
        enc->write(QByteArrayLiteral("only_apply_on_new_torrents"));
        enc->write((bt::Uint32)(policy.only_apply_on_new_torrents ? 1 : 0));
        enc->write(QByteArrayLiteral("default_move_on_completion_location"));
        enc->write(policy.default_move_on_completion_location.toUtf8());
        enc->end();
        enc->end();
    }

    void TorrentGroup::load(bt::BDictNode* dn)
    {
        name = QString::fromLocal8Bit(dn->getByteArray("name"));
        setIconByName(QString::fromLocal8Bit(dn->getByteArray("icon")));
        BListNode* ln = dn->getList("hashes");
        if (!ln)
            return;

        path = QLatin1String("/all/custom/") + name;

        for (Uint32 i = 0; i < ln->getNumChildren(); i++)
        {
            QByteArray ba = ln->getByteArray(i);
            if (ba.size() != 20)
                continue;

            hashes.insert(SHA1Hash((const Uint8*)ba.data()));
        }

        if (BDictNode* gp = dn->getDict(QByteArrayLiteral("policy")))
        {
            // load the group policy
            if (gp->getValue(QByteArrayLiteral("default_save_location")))
            {
                policy.default_save_location = gp->getString(QByteArrayLiteral("default_save_location"), 0);
                if (policy.default_save_location.length() == 0)
                    policy.default_save_location = QString(); // make sure that 0 length strings are loaded as null strings
            }

            if (gp->getValue(QByteArrayLiteral("default_move_on_completion_location")))
            {
                policy.default_move_on_completion_location = gp->getString(QByteArrayLiteral("default_move_on_completion_location"), 0);
                if (policy.default_move_on_completion_location.length() == 0)
                    policy.default_move_on_completion_location = QString(); // make sure that 0 length strings are loaded as null strings
            }

            if (gp->getValue(QByteArrayLiteral("max_share_ratio")))
                policy.max_share_ratio = gp->getString(QByteArrayLiteral("max_share_ratio"), 0).toFloat();

            if (gp->getValue(QByteArrayLiteral("max_seed_time")))
                policy.max_seed_time = gp->getString(QByteArrayLiteral("max_seed_time"), 0).toFloat();

            if (gp->getValue(QByteArrayLiteral("max_upload_rate")))
                policy.max_upload_rate = gp->getInt(QByteArrayLiteral("max_upload_rate"));

            if (gp->getValue(QByteArrayLiteral("max_download_rate")))
                policy.max_download_rate = gp->getInt(QByteArrayLiteral("max_download_rate"));

            if (gp->getValue(QByteArrayLiteral("only_apply_on_new_torrents")))
                policy.only_apply_on_new_torrents = gp->getInt(QByteArrayLiteral("only_apply_on_new_torrents"));
        }
    }

    void TorrentGroup::torrentRemoved(TorrentInterface* tor)
    {
        removeTorrent(tor);
    }

    void TorrentGroup::removeTorrent(TorrentInterface* tor)
    {
        torrents.erase(tor);
        torrentRemoved(this);
    }

    void TorrentGroup::addTorrent(TorrentInterface* tor, bool new_torrent)
    {
        torrents.insert(tor);
        // apply group policy if needed
        if (policy.only_apply_on_new_torrents && !new_torrent)
            return;

        if (bt::Exists(policy.default_move_on_completion_location))
            tor->setMoveWhenCompletedDir(policy.default_move_on_completion_location);
        tor->setMaxShareRatio(policy.max_share_ratio);
        tor->setMaxSeedTime(policy.max_seed_time);
        tor->setTrafficLimits(policy.max_upload_rate * 1024, policy.max_download_rate * 1024);

        torrentAdded(this);
    }

    void TorrentGroup::policyChanged()
    {
        if (policy.only_apply_on_new_torrents)
            return;

        std::set<TorrentInterface*>::iterator i = torrents.begin();
        while (i != torrents.end())
        {
            TorrentInterface* tor = *i;
            tor->setMaxShareRatio(policy.max_share_ratio);
            tor->setMaxSeedTime(policy.max_seed_time);
            tor->setTrafficLimits(policy.max_upload_rate * 1024, policy.max_download_rate * 1024);
            i++;
        }
    }

    void TorrentGroup::loadTorrents(QueueManager* qman)
    {
        QueueManager::iterator i = qman->begin();
        while (i != qman->end())
        {
            if (hashes.count((*i)->getInfoHash()) > 0)
                torrents.insert(*i);
            i++;
        }

        hashes.clear();
    }

}
