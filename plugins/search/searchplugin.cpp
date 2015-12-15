/***************************************************************************
 *   Copyright (C) 2005-2007 by Joris Guisson                              *
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
#include "searchplugin.h"

#include <QFile>
#include <QMenu>

#include <kpluginfactory.h>
//#include <ksharedconfig.h>

#include <klocalizedstring.h>
#include <kactioncollection.h>
#include <krun.h>
#include <kshell.h>
#include <interfaces/guiinterface.h>
#include <interfaces/coreinterface.h>
#include <interfaces/functions.h>
#include <util/log.h>
#include <util/logsystemmanager.h>
#include "searchwidget.h"
#include "searchprefpage.h"
#include "searchtoolbar.h"
#include "searchpluginsettings.h"
#include "searchenginelist.h"
#include "searchactivity.h"

K_PLUGIN_FACTORY_WITH_JSON(ktorrent_search, "ktorrent_search.json", registerPlugin<kt::SearchPlugin>();)

using namespace bt;

namespace kt
{

    SearchPlugin::SearchPlugin(QObject* parent, const QVariantList& args) : Plugin(parent), engines(0)
    {
        Q_UNUSED(args);
        pref = 0;
    }


    SearchPlugin::~SearchPlugin()
    {}


    void SearchPlugin::load()
    {
        LogSystemManager::instance().registerSystem(i18nc("plugin name", "Search"), SYS_SRC);
        engines = new SearchEngineList(kt::DataDir() + "searchengines/");
        engines->loadEngines();

        pref = new SearchPrefPage(this, engines, 0);
        getGUI()->addPrefPage(pref);
        connect(getCore(), SIGNAL(settingsChanged()), this, SLOT(preferencesUpdated()));

        activity = new SearchActivity(this, 0);
        getGUI()->addActivity(activity);
        activity->loadCurrentSearches();
        activity->loadState(KSharedConfig::openConfig());

        connect(pref, SIGNAL(clearSearchHistory()), activity, SLOT(clearSearchHistory()));
    }

    void SearchPlugin::unload()
    {
        LogSystemManager::instance().unregisterSystem(i18nc("plugin name", "Search"));
        getGUI()->removeActivity(activity);
        activity->saveCurrentSearches();
        activity->saveState(KSharedConfig::openConfig());

        getGUI()->removePrefPage(pref);
        delete pref;
        pref = 0;
        disconnect(getCore(), SIGNAL(settingsChanged()), this, SLOT(preferencesUpdated()));
        delete engines;
        engines = 0;
        delete activity;
        activity = 0;
    }

    void SearchPlugin::search(const QString& text, int engine, bool external)
    {
        if (external)
        {
            if (engine < 0 || engine >= (int)engines->getNumEngines())
                engine = 0;

            QUrl url = engines->search(engine, text);

            if (SearchPluginSettings::useDefaultBrowser())
                new KRun(url, QApplication::activeWindow());
            else
                KRun::runCommand(SearchPluginSettings::customBrowser() + ' ' + KShell::quoteArg(url.toDisplayString()), 0);
        }
        else
        {
            activity->search(text, engine);
            getGUI()->setCurrentActivity(activity);
        }
    }

    void SearchPlugin::preferencesUpdated()
    {
    }

    bool SearchPlugin::versionCheck(const QString& version) const
    {
        return version == KT_VERSION_MACRO;
    }

}

#include "searchplugin.moc"
