/***************************************************************************
*   Copyright (C) 2009 by Joris Guisson                                   *
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

#include <QFile>
#include <KLocalizedString>

#include <util/log.h>
#include <util/file.h>
#include <util/error.h>
#include <util/sha1hash.h>
#include <bcodec/bencoder.h>
#include <bcodec/bdecoder.h>
#include <bcodec/bnode.h>
#include <torrent/queuemanager.h>
#include "shutdownruleset.h"


using namespace bt;

namespace kt
{

    ShutdownRuleSet::ShutdownRuleSet(kt::CoreInterface* core, QObject* parent)
        : QObject(parent),
          core(core),
          on(false),
          all_rules_must_be_hit(false)
    {
        connect(core, &CoreInterface::torrentAdded, this, &ShutdownRuleSet::torrentAdded);
        connect(core, &CoreInterface::torrentRemoved, this, &ShutdownRuleSet::torrentRemoved);
        QueueManager* qman = core->getQueueManager();
        for (QueueManager::iterator i = qman->begin(); i != qman->end(); i++)
        {
            torrentAdded(*i);
        }
    }

    ShutdownRuleSet::~ShutdownRuleSet()
    {
    }

    void ShutdownRuleSet::clear()
    {
        rules.clear();
    }

    void ShutdownRuleSet::addRule(kt::Action action, kt::Target target, kt::Trigger trigger, bt::TorrentInterface* tc)
    {
        ShutdownRule rule;
        rule.action = action;
        rule.target = target;
        rule.trigger = trigger;
        rule.tc = tc;
        rule.hit = false;
        rules.append(rule);
    }

    void ShutdownRuleSet::torrentFinished(bt::TorrentInterface* tc)
    {
        triggered(DOWNLOADING_COMPLETED, tc);
    }

    void ShutdownRuleSet::seedingAutoStopped(bt::TorrentInterface* tc, bt::AutoStopReason reason)
    {
        Q_UNUSED(reason);
        triggered(SEEDING_COMPLETED, tc);
    }

    void ShutdownRuleSet::triggered(Trigger trigger, TorrentInterface* tc)
    {
        if (!on)
            return;

        bool hit = false;
        bool all_hit = true;
        for (QList<ShutdownRule>::iterator i = rules.begin(); i != rules.end(); i++)
        {
            bool rule_hit = false;
            if (trigger == DOWNLOADING_COMPLETED)
                rule_hit = i->downloadingFinished(tc, core->getQueueManager());
            else
                rule_hit = i->seedingFinished(tc, core->getQueueManager());

            if (rule_hit)
                hit = true;
            else if (!i->hit)
                all_hit = false;
        }

        if ((!all_rules_must_be_hit && hit) || (all_rules_must_be_hit && all_hit))
        {
            switch (currentAction())
            {
            case SHUTDOWN: emit shutdown(); break;
            case LOCK: emit lock(); break;
            case SUSPEND_TO_DISK: emit suspendToDisk(); break;
            case SUSPEND_TO_RAM: emit suspendToRAM(); break;
            }
        }
    }


    void ShutdownRuleSet::torrentAdded(bt::TorrentInterface* tc)
    {
        connect(tc, &bt::TorrentInterface::seedingAutoStopped, this, &ShutdownRuleSet::seedingAutoStopped);
        connect(tc, &bt::TorrentInterface::finished, this, &ShutdownRuleSet::torrentFinished);
    }


    void ShutdownRuleSet::torrentRemoved(bt::TorrentInterface* tc)
    {
        // Throw away all rules for this torrent
        for (QList<ShutdownRule>::iterator i = rules.begin(); i != rules.end();)
        {
            if (i->tc == tc)
                i = rules.erase(i);
            else
                i++;
        }
    }

    void ShutdownRuleSet::setEnabled(bool on)
    {
        this->on = on;
    }

    void ShutdownRuleSet::save(const QString& file)
    {
        File fptr;
        if (!fptr.open(file, QStringLiteral("wt")))
        {
            Out(SYS_GEN | LOG_DEBUG) << "Failed to open file " << file << " : " << fptr.errorString() << endl;
            return;
        }

        BEncoder enc(new BEncoderFileOutput(&fptr));
        enc.beginList();
        for (QList<ShutdownRule>::iterator i = rules.begin(); i != rules.end(); i++)
        {
            enc.beginDict();
            enc.write("Action", (bt::Uint32)i->action);
            enc.write("Trigger", (bt::Uint32)i->trigger);
            enc.write("Target", (bt::Uint32)i->target);
            if (i->target == SPECIFIC_TORRENT)
            {
                bt::SHA1Hash hash = i->tc->getInfoHash();
                enc.write(QByteArrayLiteral("Torrent"));
                enc.write(hash.getData(), 20);
            }
            enc.write(QByteArrayLiteral("hit"), i->hit);
            enc.end();
        }
        enc.write(on);
        enc.write(all_rules_must_be_hit);
        enc.end();
    }

    void ShutdownRuleSet::load(const QString& file)
    {
        QFile fptr(file);
        if (!fptr.open(QIODevice::ReadOnly))
        {
            Out(SYS_GEN | LOG_DEBUG) << "Failed to open file " << file << " : " << fptr.errorString() << endl;
            return;
        }

        QByteArray data = fptr.readAll();
        BDecoder dec(data, false);
        BNode* node = 0;
        try
        {
            clear();
            node = dec.decode();
            if (!node || node->getType() != BNode::LIST)
                throw bt::Error(QStringLiteral("Toplevel node not a list"));

            BListNode* const l = (BListNode*)node;
            Uint32 i = 0;
            for (; i < l->getNumChildren(); ++i)
            {
                if (l->getChild(i)->getType() != BNode::DICT)
                    break;

                BDictNode* const d = l->getDict(i);
                if (!d)
                    continue;

                ShutdownRule rule;
                rule.action = (Action)d->getInt(QByteArrayLiteral("Action"));
                rule.target = (Target)d->getInt(QByteArrayLiteral("Target"));
                rule.trigger = (Trigger)d->getInt(QByteArrayLiteral("Trigger"));
                rule.hit = d->keys().contains(QByteArrayLiteral("hit")) && d->getInt(QByteArrayLiteral("hit")) == 1;
                rule.tc = 0;
                if (d->getValue(QByteArrayLiteral("Torrent")))
                {
                    const QByteArray hash = d->getByteArray(QByteArrayLiteral("Torrent"));
                    bt::TorrentInterface* const tc = torrentForHash(hash);
                    if (tc)
                        rule.tc = tc;
                    else
                        continue; // no valid torrent found so skip this rule
                }
                rules.append(rule);
            }

            on = (l->getInt(i++) == 1);
            if (i < l->getNumChildren())
                all_rules_must_be_hit = (l->getInt(i) == 1);
            else
                all_rules_must_be_hit = false;
        }
        catch (bt::Error& err)
        {
            Out(SYS_GEN | LOG_DEBUG) << "Failed to parse " << file << " : " << err.toString() << endl;
        }

        delete node;
    }

    bt::TorrentInterface* ShutdownRuleSet::torrentForHash(const QByteArray& hash)
    {
        bt::SHA1Hash ih((const bt::Uint8*)hash.data());
        QueueManager* qman = core->getQueueManager();
        for (QueueManager::iterator i = qman->begin(); i != qman->end(); i++)
        {
            bt::TorrentInterface* t = *i;
            if (t->getInfoHash() == ih)
                return t;
        }

        return 0;
    }

    kt::Action ShutdownRuleSet::currentAction() const
    {
        if (rules.count() == 0)
            return SHUTDOWN;
        else
            return rules.front().action;
    }

    QString ShutdownRuleSet::toolTip() const
    {
        if (rules.isEmpty())
        {
            return i18n("Automatic shutdown not active");
        }
        else
        {
            QString msg;
            Action action = currentAction();
            switch (action)
            {
            case SHUTDOWN:
                msg = i18n("Shutdown");
                break;
            case LOCK:
                msg = i18n("Lock");
                break;
            case SUSPEND_TO_RAM:
                msg = i18n("Sleep (suspend to RAM)");
                break;
            case SUSPEND_TO_DISK:
                msg = i18n("Hibernate (suspend to disk)");
                break;
            }


            if (all_rules_must_be_hit)
                msg += i18n(" when all of the following events have occurred:<br/><br/> ");
            else
                msg += i18n(" when one of the following events occur:<br/><br/> ");

            QStringList items;
            for (const ShutdownRule& r : qAsConst(rules))
            {
                items += QStringLiteral("- ") + r.toolTip();
            }


            msg += items.join(QStringLiteral("<br/>"));
            return msg;
        }
    }



    //////////////////////////////////////

    bool ShutdownRule::downloadingFinished(bt::TorrentInterface* tor, QueueManager* qman)
    {
        if (target != ALL_TORRENTS && tc != tor)
            return false;

        if (trigger != DOWNLOADING_COMPLETED)
            return false;

        if (target != ALL_TORRENTS)
        {
            hit = tc == tor;
            return hit;
        }
        else
        {
            // target is all torrents, so check if all torrents have completed downloading
            for (QueueManager::iterator i = qman->begin(); i != qman->end(); i++)
            {
                bt::TorrentInterface* t = *i;
                const bt::TorrentStats& stats = t->getStats();
                if (t != tor && !stats.completed && stats.running)
                    return false;
            }

            hit = true;
            return true;
        }
    }

    bool ShutdownRule::seedingFinished(bt::TorrentInterface* tor, QueueManager* qman)
    {
        if (target != ALL_TORRENTS && tc != tor)
            return false;

        if (trigger != SEEDING_COMPLETED)
            return false;

        if (target != ALL_TORRENTS)
        {
            hit = tc == tor;
            return hit;
        }
        else
        {
            // target is all torrents, so check if all torrents have completed seeding
            for (QueueManager::iterator i = qman->begin(); i != qman->end(); i++)
            {
                bt::TorrentInterface* t = *i;
                if (t == tor)
                    continue;

                const bt::TorrentStats& stats = t->getStats();
                if (stats.running)
                    return false;
            }

            hit = true;
            return true;
        }
    }

    QString ShutdownRule::toolTip() const
    {
        if (target == ALL_TORRENTS && trigger == kt::DOWNLOADING_COMPLETED)
            return i18n("<b>All torrents</b> finish downloading");
        else if (target == ALL_TORRENTS && trigger == kt::SEEDING_COMPLETED)
            return i18n("<b>All torrents</b> finish seeding");
        else if (target == SPECIFIC_TORRENT && trigger == kt::DOWNLOADING_COMPLETED)
            return i18n("<b>%1</b> finishes downloading", tc->getDisplayName());
        else if (target == SPECIFIC_TORRENT && trigger == kt::SEEDING_COMPLETED)
            return i18n("<b>%1</b> finishes seeding", tc->getDisplayName());
        else
            return QString();
    }



}

