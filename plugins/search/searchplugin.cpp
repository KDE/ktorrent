/*
    SPDX-FileCopyrightText: 2005-2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "searchplugin.h"

#include <QFile>
#include <QMenu>

#include <KActionCollection>
#include <KIO/CommandLauncherJob>
#include <KIO/OpenUrlJob>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KSharedConfig>
#include <KShell>

#include "searchactivity.h"
#include "searchenginelist.h"
#include "searchpluginsettings.h"
#include "searchprefpage.h"
#include "searchtoolbar.h"
#include "searchwidget.h"
#include <dbus/dbus.h>
#include <interfaces/coreinterface.h>
#include <interfaces/functions.h>
#include <interfaces/guiinterface.h>
#include <util/log.h>
#include <util/logsystemmanager.h>

K_PLUGIN_FACTORY_WITH_JSON(ktorrent_search, "ktorrent_search.json", registerPlugin<kt::SearchPlugin>();)

using namespace bt;

namespace kt
{
SearchPlugin::SearchPlugin(QObject *parent, const QVariantList &args)
    : Plugin(parent)
    , engines(nullptr)
{
    Q_UNUSED(args);
    pref = nullptr;
}

SearchPlugin::~SearchPlugin()
{
}

void SearchPlugin::load()
{
    LogSystemManager::instance().registerSystem(i18nc("plugin name", "Search"), SYS_SRC);
    proxy = new ProxyHelper((DBusSettings *)getCore()->getExternalInterface()->settings());
    engines = new SearchEngineList(proxy, kt::DataDir() + QStringLiteral("searchengines/"));
    engines->loadEngines();

    pref = new SearchPrefPage(this, engines, nullptr);
    getGUI()->addPrefPage(pref);
    connect(getCore(), &CoreInterface::settingsChanged, this, &SearchPlugin::preferencesUpdated);

    activity = new SearchActivity(this, nullptr);
    getGUI()->addActivity(activity);
    activity->loadCurrentSearches();
    activity->loadState(KSharedConfig::openConfig());

    connect(pref, &SearchPrefPage::clearSearchHistory, activity, &SearchActivity::clearSearchHistory);
}

void SearchPlugin::unload()
{
    LogSystemManager::instance().unregisterSystem(i18nc("plugin name", "Search"));
    getGUI()->removeActivity(activity);
    activity->saveCurrentSearches();
    activity->saveState(KSharedConfig::openConfig());

    getGUI()->removePrefPage(pref);
    delete pref;
    pref = nullptr;
    connect(getCore(), &CoreInterface::settingsChanged, this, &SearchPlugin::preferencesUpdated);
    delete engines;
    engines = nullptr;
    delete activity;
    activity = nullptr;
    delete proxy;
    proxy = nullptr;
}

void SearchPlugin::search(const QString &text, int engine, bool external)
{
    if (external) {
        if (engine < 0 || engine >= (int)engines->getNumEngines())
            engine = 0;

        QUrl url = engines->search(engine, text);

        if (SearchPluginSettings::useDefaultBrowser()) {
            auto *job = new KIO::OpenUrlJob(url, QApplication::activeWindow());
            job->start();
        } else {
            auto *job =
                new KIO::CommandLauncherJob(SearchPluginSettings::customBrowser() + QStringLiteral(" ") + KShell::quoteArg(url.toDisplayString()), nullptr);
            job->start();
        }
    } else {
        activity->search(text, engine);
        getGUI()->setCurrentActivity(activity);
    }
}

void SearchPlugin::preferencesUpdated()
{
}

bool SearchPlugin::versionCheck(const QString &version) const
{
    return version == QStringLiteral(VERSION);
}

}

#include "searchplugin.moc"
