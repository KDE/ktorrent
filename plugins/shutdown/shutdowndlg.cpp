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

#include "shutdowndlg.h"

#include <KConfigGroup>
#include <QVBoxLayout>

#include "shutdowntorrentmodel.h"
#include "powermanagement_interface.h"

namespace kt
{
    ShutdownDlg::ShutdownDlg(ShutdownRuleSet* rules, CoreInterface* core, QWidget* parent) : QDialog(parent), rules(rules)
    {
        setupUi(this);
        connect(buttonBox, &QDialogButtonBox::accepted, this, &ShutdownDlg::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &ShutdownDlg::reject);
        setWindowTitle(i18nc("@title:window", "Configure Shutdown"));
        model = new ShutdownTorrentModel(core, this);

        m_action->addItem(QIcon::fromTheme(QStringLiteral("system-shutdown")), i18n("Shutdown"), SHUTDOWN);
        m_action->addItem(QIcon::fromTheme(QStringLiteral("system-lock-screen")), i18n("Lock"), LOCK);

        org::freedesktop::PowerManagement powerManagement(QStringLiteral("org.freedesktop.PowerManagement"), QStringLiteral("/org/freedesktop/PowerManagement"), QDBusConnection::sessionBus());

        auto pendingSuspendReply = powerManagement.CanSuspend();
        pendingSuspendReply.waitForFinished();
        if (!pendingSuspendReply.isError())
            if (pendingSuspendReply.value())
                m_action->addItem(QIcon::fromTheme(QStringLiteral("system-suspend")), i18n("Sleep (suspend to RAM)"), SUSPEND_TO_RAM);

        auto pendingHibernateReply = powerManagement.CanHibernate();
        pendingHibernateReply.waitForFinished();
        if (!pendingHibernateReply.isError())
            if (pendingHibernateReply.value())
                m_action->addItem(QIcon::fromTheme(QStringLiteral("system-suspend-hibernate")), i18n("Hibernate (suspend to disk)"), SUSPEND_TO_DISK);

        m_time_to_execute->addItem(i18n("When all torrents finish downloading"));
        m_time_to_execute->addItem(i18n("When all torrents finish seeding"));
        m_time_to_execute->addItem(i18n("When the events below happen"));
        m_all_rules_must_be_hit->setChecked(rules->allRulesMustBeHit());

        connect(m_time_to_execute, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &ShutdownDlg::timeToExecuteChanged);
        m_torrent_list->setEnabled(false);
        m_torrent_list->setModel(model);
        m_torrent_list->setRootIsDecorated(false);
        m_torrent_list->setItemDelegateForColumn(1, new ShutdownTorrentDelegate(this));

        for (int i = 0; i < rules->count(); i++)
        {
            const ShutdownRule& r = rules->rule(i);
            if (r.target == ALL_TORRENTS)
            {
                m_action->setCurrentIndex(actionToIndex(r.action));
                m_time_to_execute->setCurrentIndex(r.trigger == DOWNLOADING_COMPLETED ? 0 : 1);
            }
            else
            {
                m_action->setCurrentIndex(actionToIndex(r.action));
                m_time_to_execute->setCurrentIndex(2);
                model->addRule(r);
            }
        }

        m_all_rules_must_be_hit->setEnabled(m_time_to_execute->currentIndex() == 2);
    }

    ShutdownDlg::~ShutdownDlg()
    {

    }

    void ShutdownDlg::accept()
    {
        rules->setAllRulesMustBeHit(m_all_rules_must_be_hit->isChecked());
        if (m_time_to_execute->currentIndex() == 2)
        {
            model->applyRules(indexToAction(m_action->currentIndex()), rules);
        }
        else
        {
            rules->clear();
            Trigger trigger = m_time_to_execute->currentIndex() == 0 ? DOWNLOADING_COMPLETED : SEEDING_COMPLETED;
            rules->addRule(indexToAction(m_action->currentIndex()), ALL_TORRENTS, trigger);
        }
        QDialog::accept();
    }

    void ShutdownDlg::reject()
    {
        QDialog::reject();
    }

    void ShutdownDlg::timeToExecuteChanged(int idx)
    {
        m_torrent_list->setEnabled(idx == 2);
        m_all_rules_must_be_hit->setEnabled(idx == 2);
    }

    kt::Action ShutdownDlg::indexToAction(int idx)
    {
        int suspend_to_ram = m_action->findData(SUSPEND_TO_RAM);
        int suspend_to_disk = m_action->findData(SUSPEND_TO_DISK);

        if (idx == 0)
            return SHUTDOWN;
        else if (idx == 1)
            return LOCK;
        else if (idx == suspend_to_ram)
            return SUSPEND_TO_RAM;
        else if (idx == suspend_to_disk)
            return SUSPEND_TO_DISK;
        else
            return SHUTDOWN;
    }

    int ShutdownDlg::actionToIndex(Action act)
    {
        int suspend_to_ram = m_action->findData(SUSPEND_TO_RAM);
        int suspend_to_disk = m_action->findData(SUSPEND_TO_DISK);

        switch (act)
        {
        case SHUTDOWN: return 0;
        case LOCK: return 1;
        case SUSPEND_TO_RAM: return suspend_to_ram;
        case SUSPEND_TO_DISK: return suspend_to_disk;
        default:
            return -1;
        }
    }

}
