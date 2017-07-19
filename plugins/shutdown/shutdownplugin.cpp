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

#include "shutdownplugin.h"

#include <KActionCollection>
#include <KJob>
#include <KMessageBox>
#include <KPluginFactory>
#include <KToggleAction>
#include <kworkspace.h>

#include <util/log.h>
#include <interfaces/functions.h>
#include "screensaver_interface.h"
#include "shutdowndlg.h"
#include "shutdownruleset.h"
#include "powermanagement_interface.h"

K_PLUGIN_FACTORY_WITH_JSON(ktorrent_shutdown, "ktorrent_shutdown.json", registerPlugin<kt::ShutdownPlugin>();)

using namespace bt;

namespace kt
{
    ShutdownPlugin::ShutdownPlugin(QObject* parent, const QVariantList& args) : Plugin(parent)
    {
        Q_UNUSED(args);

        KActionCollection* ac = actionCollection();
        shutdown_enabled = new KToggleAction(QIcon::fromTheme(QStringLiteral("system-shutdown")), i18n("Shutdown Enabled"), this);
        connect(shutdown_enabled, &KToggleAction::toggled, this, &ShutdownPlugin::shutdownToggled);
        ac->addAction(QStringLiteral("shutdown_enabled"), shutdown_enabled);

        configure_shutdown = new QAction(QIcon::fromTheme(QStringLiteral("preferences-other")), i18n("Configure Shutdown"), this);
        connect(configure_shutdown, &QAction::triggered, this, &ShutdownPlugin::configureShutdown);
        ac->addAction(QStringLiteral("shutdown_settings"), configure_shutdown);

        setXMLFile(QStringLiteral("ktorrent_shutdownui.rc"));
    }

    ShutdownPlugin::~ShutdownPlugin()
    {
    }

    bool ShutdownPlugin::versionCheck(const QString& version) const
    {
        return version == QStringLiteral(KT_VERSION_MACRO);
    }

    void ShutdownPlugin::unload()
    {
        rules->save(kt::DataDir() + QStringLiteral("shutdown_rules"));
        delete rules;
        rules = nullptr;
    }

    void ShutdownPlugin::load()
    {
        rules = new ShutdownRuleSet(getCore(), this);
        rules->load(kt::DataDir() + QStringLiteral("shutdown_rules"));
        if (rules->enabled())
            shutdown_enabled->setChecked(true);
        connect(rules, &ShutdownRuleSet::shutdown, this, &ShutdownPlugin::shutdownComputer);
        connect(rules, &ShutdownRuleSet::lock, this, &ShutdownPlugin::lock);
        connect(rules, &ShutdownRuleSet::suspendToDisk, this, &ShutdownPlugin::suspendToDisk);
        connect(rules, &ShutdownRuleSet::suspendToRAM, this, &ShutdownPlugin::suspendToRam);
        updateAction();
    }

    void ShutdownPlugin::shutdownComputer()
    {
        Out(SYS_GEN | LOG_NOTICE) << "Shutting down computer ..." << endl;
        KWorkSpace::requestShutDown(KWorkSpace::ShutdownConfirmYes, KWorkSpace::ShutdownTypeHalt);
    }

    void ShutdownPlugin::lock()
    {
        Out(SYS_GEN | LOG_NOTICE) << "Locking screen ..." << endl;
        QString interface(QStringLiteral("org.freedesktop.ScreenSaver"));
        org::freedesktop::ScreenSaver screensaver(interface, QStringLiteral("/ScreenSaver"), QDBusConnection::sessionBus());
        screensaver.Lock();
    }

    void ShutdownPlugin::suspendToDisk()
    {
        org::freedesktop::PowerManagement powerManagement(QStringLiteral("org.freedesktop.PowerManagement"), QStringLiteral("/org/freedesktop/PowerManagement"), QDBusConnection::sessionBus());
        Out(SYS_GEN | LOG_NOTICE) << "Suspending to disk ..." << endl;
        powerManagement.Hibernate();
   }

    void ShutdownPlugin::suspendToRam()
    {
        org::freedesktop::PowerManagement powerManagement(QStringLiteral("org.freedesktop.PowerManagement"), QStringLiteral("/org/freedesktop/PowerManagement"), QDBusConnection::sessionBus());
        Out(SYS_GEN | LOG_NOTICE) << "Suspending to RAM ..." << endl;
        powerManagement.Suspend();
    }

    void ShutdownPlugin::shutdownToggled(bool on)
    {
        if (on && !rules->valid())
        {
            configureShutdown();
            if (rules->valid())
                rules->setEnabled(on);
            else
                shutdown_enabled->setChecked(false);
        }
        else
            rules->setEnabled(on);
    }

    void ShutdownPlugin::configureShutdown()
    {
        ShutdownDlg dlg(rules, getCore(), nullptr);
        if (dlg.exec() == QDialog::Accepted)
        {
            rules->save(kt::DataDir() + QLatin1String("shutdown_rules"));
            updateAction();
        }
    }

    void ShutdownPlugin::updateAction()
    {
        switch (rules->currentAction())
        {
        case SHUTDOWN:
            shutdown_enabled->setIcon(QIcon::fromTheme(QLatin1String("system-shutdown")));
            shutdown_enabled->setText(i18n("Shutdown"));
            break;
        case LOCK:
            shutdown_enabled->setIcon(QIcon::fromTheme(QLatin1String("system-lock-screen")));
            shutdown_enabled->setText(i18n("Lock"));
            break;
        case SUSPEND_TO_RAM:
            shutdown_enabled->setIcon(QIcon::fromTheme(QLatin1String("system-suspend")));
            shutdown_enabled->setText(i18n("Sleep (suspend to RAM)"));
            break;
        case SUSPEND_TO_DISK:
            shutdown_enabled->setIcon(QIcon::fromTheme(QLatin1String("system-suspend-hibernate")));
            shutdown_enabled->setText(i18n("Hibernate (suspend to disk)"));
            break;
        }

        shutdown_enabled->setToolTip(rules->toolTip());
    }

}

#include "shutdownplugin.moc"
