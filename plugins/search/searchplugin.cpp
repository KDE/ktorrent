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
#include <QFile>
#include <kgenericfactory.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kactioncollection.h>
#include <kstdaction.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <krun.h>
#include <QMenu>
#include <kshell.h>
#include <interfaces/guiinterface.h>
#include <interfaces/coreinterface.h>
#include <interfaces/functions.h>
#include <util/log.h>
#include <util/logsystemmanager.h>
#include "searchplugin.h"
#include "searchwidget.h"
#include "searchprefpage.h"
#include "searchtoolbar.h"
#include "searchpluginsettings.h"
#include "searchenginelist.h"
#include "searchactivity.h"

K_EXPORT_COMPONENT_FACTORY(ktsearchplugin, KGenericFactory<kt::SearchPlugin>("ktsearchplugin"))

using namespace bt;

namespace kt
{

    SearchPlugin::SearchPlugin(QObject* parent, const QStringList& args) : Plugin(parent), engines(0)
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
        activity->loadState(KGlobal::config());

        connect(pref, SIGNAL(clearSearchHistory()), activity, SLOT(clearSearchHistory()));
    }

    void SearchPlugin::unload()
    {
        LogSystemManager::instance().unregisterSystem(i18nc("plugin name", "Search"));
        getGUI()->removeActivity(activity);
        activity->saveCurrentSearches();
        activity->saveState(KGlobal::config());

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

            KUrl url = engines->search(engine, text);

            if (SearchPluginSettings::useDefaultBrowser())
                new KRun(KUrl(url), QApplication::activeWindow());
            else
                KRun::runCommand(QString("%1 %2").arg(SearchPluginSettings::customBrowser()).arg(KShell::quoteArg(url.url())), 0);
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
