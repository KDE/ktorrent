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

#ifndef KT_SHUTDOWNRULESET_H
#define KT_SHUTDOWNRULESET_H

#include <QObject>
#include <interfaces/coreinterface.h>
#include <interfaces/torrentinterface.h>

namespace kt
{
    class QueueManager;


    enum Trigger
    {
        DOWNLOADING_COMPLETED = 0,
        SEEDING_COMPLETED
    };

    enum Target
    {
        ALL_TORRENTS,
        SPECIFIC_TORRENT
    };

    enum Action
    {
        SHUTDOWN,
        LOCK,
        SUSPEND_TO_DISK,
        SUSPEND_TO_RAM
    };

    struct ShutdownRule
    {
        Trigger trigger;
        Target target;
        Action action;
        bt::TorrentInterface* tc;
        bool hit;

        bool downloadingFinished(bt::TorrentInterface* tor, QueueManager* qman);
        bool seedingFinished(bt::TorrentInterface* tor, QueueManager* qman);
        QString toolTip() const;
    };

    /**
        Keeps track of all shutdown rules and monitors for events (torrent finished with seeding or downloading).
        When an event is received, it will determine the course of action.
    */
    class ShutdownRuleSet : public QObject
    {
        Q_OBJECT
    public:
        ShutdownRuleSet(CoreInterface* core, QObject* parent);
        ~ShutdownRuleSet();

        /// Set if all rules must be hit, before actions are undertaken
        void setAllRulesMustBeHit(bool on) {all_rules_must_be_hit = on;}

        /// See if all rules must be hit, before actions are undertaken
        bool allRulesMustBeHit() const {return all_rules_must_be_hit;}

        /// Get the current action
        Action currentAction() const;

        /// Do we have a valid ruleset
        bool valid() const {return !rules.isEmpty();}

        /// Enable or disable the rules
        void setEnabled(bool on);

        /// Whether or not the rules are enabled
        bool enabled() const {return on;}

        /**
            Add a rule to the ruleset
        */
        void addRule(Action action, Target target, Trigger trigger, bt::TorrentInterface* tc = 0);

        /**
            Clear all rules
        */
        void clear();

        /// Get the number of rules
        int count() const {return rules.count();}

        /// Get a rule
        const ShutdownRule& rule(int idx) const {return rules.at(idx);}

        /// Save the ruleset to a file
        void save(const QString& file);

        /// Load the ruleset from a file
        void load(const QString& file);

        /// Generate a tool tip for the current ruleset
        QString toolTip() const;
    signals:
        void shutdown();
        void standby();
        void lock();
        void suspendToDisk();
        void suspendToRAM();

    private slots:
        void torrentFinished(bt::TorrentInterface* tc);
        void seedingAutoStopped(bt::TorrentInterface* tc, bt::AutoStopReason reason);
        void torrentAdded(bt::TorrentInterface* tc);
        void torrentRemoved(bt::TorrentInterface* tc);

    private:
        bt::TorrentInterface* torrentForHash(const QByteArray& hash);
        void triggered(Trigger trigger, bt::TorrentInterface* tc);

    private:
        QList<ShutdownRule> rules;
        CoreInterface* core;
        bool on;
        bool all_rules_must_be_hit;
    };

}

#endif // KT_SHUTDOWNRULESET_H
