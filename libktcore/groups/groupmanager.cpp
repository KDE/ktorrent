/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
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

#include "groupmanager.h"

#include <KLocalizedString>

#include <util/log.h>
#include <util/file.h>
#include <util/fileops.h>
#include <util/error.h>
#include <bcodec/bnode.h>
#include <bcodec/bdecoder.h>
#include <bcodec/bencoder.h>
#include <interfaces/functions.h>
#include <interfaces/torrentinterface.h>
#include "torrentgroup.h"
#include "allgroup.h"
#include "torrentgroup.h"
#include "ungroupedgroup.h"
#include "functiongroup.h"

using namespace bt;

namespace kt
{

    bool upload(TorrentInterface* tor)
    {
        return tor->getStats().completed;
    }

    bool download(TorrentInterface* tor)
    {
        return !tor->getStats().completed;
    }

    bool queued(TorrentInterface* tor)
    {
        return tor->getStats().status == bt::QUEUED;
    }

    bool stalled(TorrentInterface* tor)
    {
        return tor->getStats().status == bt::STALLED;
    }

    bool error(TorrentInterface* tor)
    {
        return tor->getStats().status == bt::ERROR;
    }

    bool not_running(TorrentInterface* tor)
    {
        return tor->getStats().running == false;
    }

    bool running(TorrentInterface* tor)
    {
        return tor->getStats().running == true;
    }

    bool active(TorrentInterface* tor)
    {
        const bt::TorrentStats& s = tor->getStats();
        return (s.upload_rate >= 100 || s.download_rate >= 100);
    }

    bool passive(TorrentInterface* tor)
    {
        return !active(tor);
    }

    template <IsMemberFunction A, IsMemberFunction B>
    bool member(TorrentInterface* tor)
    {
        return A(tor) && B(tor);
    }

    GroupManager::GroupManager()
    {
        groups.setAutoDelete(true);

        all = new AllGroup();
        groups.insert(all->groupName(), all);

        QList<Group*> defaults;
        // uploads tree
        defaults << new FunctionGroup<upload>(i18n("Uploads"), QStringLiteral("go-up"), Group::UPLOADS_ONLY_GROUP, QStringLiteral("/all/uploads"));
        defaults << new FunctionGroup<member<running, upload> >(
                     i18n("Running Uploads"), QStringLiteral("kt-start"), Group::UPLOADS_ONLY_GROUP, QStringLiteral("/all/uploads/running"));
        defaults << new FunctionGroup<member<not_running, upload> >(
                     i18n("Not Running Uploads"), QStringLiteral("kt-stop"), Group::UPLOADS_ONLY_GROUP, QStringLiteral("/all/uploads/not_running"));


        // downloads tree
        defaults << new FunctionGroup<download>(i18n("Downloads"), QStringLiteral("go-down"), Group::DOWNLOADS_ONLY_GROUP, QStringLiteral("/all/downloads"));
        defaults << new FunctionGroup<member<running, download> >(
                     i18n("Running Downloads"), QStringLiteral("kt-start"), Group::DOWNLOADS_ONLY_GROUP, QStringLiteral("/all/downloads/running"));
        defaults << new FunctionGroup<member<not_running, download> >(
                     i18n("Not Running Downloads"), QStringLiteral("kt-stop"), Group::DOWNLOADS_ONLY_GROUP, QStringLiteral("/all/downloads/not_running"));


        defaults << new FunctionGroup<active>(
                     i18n("Active Torrents"), QStringLiteral("network-connect"), Group::MIXED_GROUP, QStringLiteral("/all/active"));
        defaults << new FunctionGroup<member<active, download> >(
                     i18n("Active Downloads"), QStringLiteral("go-down"), Group::DOWNLOADS_ONLY_GROUP, QStringLiteral("/all/active/downloads"));
        defaults << new FunctionGroup<member<active, upload> >(
                     i18n("Active Uploads"), QStringLiteral("go-up"), Group::UPLOADS_ONLY_GROUP, QStringLiteral("/all/active/uploads"));

        defaults << new FunctionGroup<passive>(i18n("Passive Torrents"), QStringLiteral("network-disconnect"), Group::MIXED_GROUP, QStringLiteral("/all/passive"));
        defaults << new FunctionGroup<member<passive, download> >(
                     i18n("Passive Downloads"), QStringLiteral("go-down"), Group::DOWNLOADS_ONLY_GROUP, QStringLiteral("/all/passive/downloads"));
        defaults << new FunctionGroup<member<passive, upload> >(
                     i18n("Passive Uploads"), QStringLiteral("go-up"), Group::UPLOADS_ONLY_GROUP, QStringLiteral("/all/passive/uploads"));
        defaults << new UngroupedGroup(this);

        for (Group* g : qAsConst(defaults))
            groups.insert(g->groupName(), g);
    }


    GroupManager::~GroupManager()
    {
    }


    Group* GroupManager::newGroup(const QString& name)
    {
        if (groups.find(name))
            return 0;

        TorrentGroup* g = new TorrentGroup(name);
        connect(g, &TorrentGroup::torrentAdded, this, &GroupManager::customGroupChanged);
        connect(g, static_cast<void (TorrentGroup::*)(Group*)>(&TorrentGroup::torrentRemoved), this, &GroupManager::customGroupChanged);
        groups.insert(name, g);
        emit groupAdded(g);
        return g;
    }

    void GroupManager::removeGroup(Group* g)
    {
        if (canRemove(g))
        {
            emit groupRemoved(g);
            groups.setAutoDelete(false);
            groups.erase(g->groupName());
            groups.setAutoDelete(true);
            g->deleteLater();
        }
    }

    bool GroupManager::canRemove(const Group* g) const
    {
        return g->groupFlags() & Group::CUSTOM_GROUP;
    }

    Group* GroupManager::find(const QString& name)
    {
        return groups.find(name);
    }

    QStringList GroupManager::customGroupNames()
    {
        QStringList groupNames;
        Itr it = groups.begin();

        while (it != end())
        {
            if (it->second->groupFlags() & Group::CUSTOM_GROUP)
                groupNames << it->first;
            ++it;
        }

        return groupNames;
    }

    void GroupManager::saveGroups()
    {
        QString fn = kt::DataDir() + QStringLiteral("groups");
        bt::File fptr;
        if (!fptr.open(fn, QStringLiteral("wb")))
        {
            bt::Out(SYS_GEN | LOG_DEBUG) << "Cannot open " << fn << " : " << fptr.errorString() << bt::endl;
            return;
        }

        try
        {
            bt::BEncoder enc(&fptr);

            enc.beginList();
            for (Itr i = groups.begin(); i != groups.end(); i++)
            {
                if (i->second->groupFlags() & Group::CUSTOM_GROUP)
                    i->second->save(&enc);
            }
            enc.end();
        }
        catch (bt::Error& err)
        {
            bt::Out(SYS_GEN | LOG_DEBUG) << "Error : " << err.toString() << endl;
            return;
        }
    }



    void GroupManager::loadGroups()
    {
        QString fn = kt::DataDir() + QStringLiteral("groups");
        bt::File fptr;
        if (!fptr.open(fn, QStringLiteral("rb")))
        {
            bt::Out(SYS_GEN | LOG_DEBUG) << "Cannot open " << fn << " : " << fptr.errorString() << bt::endl;
            return;
        }

        bt::BNode* n = 0;
        try
        {
            Uint32 fs = bt::FileSize(fn);
            QByteArray data(fs, 0);
            fptr.read(data.data(), fs);

            BDecoder dec(data, false);
            n = dec.decode();
            if (!n || n->getType() != bt::BNode::LIST)
                throw bt::Error(QStringLiteral("groups file corrupt"));

            BListNode* ln = (BListNode*)n;
            for (Uint32 i = 0; i < ln->getNumChildren(); i++)
            {
                BDictNode* dn = ln->getDict(i);
                if (!dn)
                    continue;

                TorrentGroup* g = new TorrentGroup(QStringLiteral("dummy"));
                connect(g, &TorrentGroup::torrentAdded, this, &GroupManager::customGroupChanged);
                connect(g, static_cast<void (TorrentGroup::*)(Group*)>(&TorrentGroup::torrentRemoved), this, &GroupManager::customGroupChanged);

                try
                {
                    g->load(dn);
                }
                catch (...)
                {
                    delete g;
                    throw;
                }

                if (!find(g->groupName()))
                    groups.insert(g->groupName(), g);
                else
                    delete g;
            }

            delete n;
        }
        catch (bt::Error& err)
        {
            bt::Out(SYS_GEN | LOG_DEBUG) << "Error : " << err.toString() << endl;
            delete n;
            return;
        }
    }

    void GroupManager::torrentRemoved(TorrentInterface* ti)
    {
        for (Itr i = groups.begin(); i != groups.end(); i++)
        {
            i->second->torrentRemoved(ti);
        }
    }

    void GroupManager::renameGroup(const QString& old_name, const QString& new_name)
    {
        QString oldName = old_name;
        Group* g = find(old_name);
        if (!g)
            return;

        groups.setAutoDelete(false);
        groups.erase(old_name);
        g->rename(new_name);
        groups.insert(new_name, g);
        groups.setAutoDelete(true);
        saveGroups();

        emit groupRenamed(g);
    }

    void GroupManager::addDefaultGroup(Group* g)
    {
        if (find(g->groupName()))
            return;

        groups.insert(g->groupName(), g);
        emit groupAdded(g);
    }


    void GroupManager::removeDefaultGroup(Group* g)
    {
        if (g)
        {
            groupRemoved(g);
            groups.erase(g->groupName());
        }
    }

    void GroupManager::torrentsLoaded(QueueManager* qman)
    {
        for (Itr i = groups.begin(); i != groups.end(); i++)
        {
            if (i->second->groupFlags() & Group::CUSTOM_GROUP)
            {
                TorrentGroup* tg = dynamic_cast<TorrentGroup*>(i->second);
                if (tg)
                    tg->loadTorrents(qman);
            }
        }
    }

    Group* GroupManager::findByPath(const QString& path)
    {
        for (Itr i = groups.begin(); i != groups.end(); i++)
        {
            if (i->second->groupPath() == path)
                return i->second;
        }

        return nullptr;
    }

    void GroupManager::updateCount(QueueManager* qman)
    {
        for (Itr i = groups.begin(); i != groups.end(); i++)
            i->second->updateCount(qman);
    }


}
