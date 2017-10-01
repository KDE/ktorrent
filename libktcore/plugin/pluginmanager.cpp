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

#include "pluginmanager.h"

#include <QFile>
#include <QTextStream>

#include <KLocalizedString>
#include <KSharedConfig>
#include <KPluginMetaData>

#include <util/log.h>
#include <util/error.h>
#include <util/fileops.h>
#include <util/waitjob.h>
#include <torrent/globals.h>
#include <interfaces/guiinterface.h>
#include "pluginactivity.h"

using namespace bt;

namespace kt
{

    PluginManager::PluginManager(CoreInterface* core, GUIInterface* gui) : core(core), gui(gui)
    {
        prefpage = 0;
        loaded.setAutoDelete(true);
    }

    PluginManager::~PluginManager()
    {
        delete prefpage;
    }

    void PluginManager::loadPluginList()
    {
        pluginsMetaData = KPluginLoader::findPlugins(QStringLiteral("ktorrent"));
        if (pluginsMetaData.isEmpty())
        {
            //simple workaround for the situation i have in debian --Nick
            QStringList paths = QCoreApplication::libraryPaths();
            if (paths.isEmpty())
                paths << QLatin1String("/usr/lib/x86_64-linux-gnu/plugins");

            QCoreApplication::addLibraryPath(paths.first().remove(QLatin1String("qt5/")));
            pluginsMetaData = KPluginLoader::findPlugins(QStringLiteral("ktorrent"));
        }

        for (const KPluginMetaData &module : pluginsMetaData)
        {
            KPluginInfo pi(module);
            pi.setConfig(KSharedConfig::openConfig()->group(pi.pluginName()));
            pi.load();

            plugins << pi;
        }

        if (!prefpage)
        {
            prefpage = new PluginActivity(this);
            gui->addActivity(prefpage);
        }

        prefpage->updatePluginList();
        loadPlugins();
        prefpage->update();
    }

    void PluginManager::loadPlugins()
    {
        int idx = 0;
        for (auto i = plugins.begin(); i != plugins.end(); i++)
        {
            KPluginInfo& pi = *i;
            if (loaded.contains(idx) & !pi.isPluginEnabled())
            {
                // unload it
                unload(pi, idx);
                pi.save();
            }
            else if (!loaded.contains(idx) && pi.isPluginEnabled())
            {
                // load it
                load(pi, idx);
                pi.save();
            }
            idx++;
        }
    }

    void PluginManager::load(const KPluginInfo& pi, int idx)
    {
        Q_UNUSED(pi)
        KPluginLoader loader(pluginsMetaData.at(idx).fileName());
        KPluginFactory *factory = loader.factory();
        if (!factory)
            return;

        Plugin* plugin = factory->create<kt::Plugin>();
        if (!plugin)
        {
            Out(SYS_GEN | LOG_NOTICE) <<
                                      QStringLiteral("Creating instance of plugin %1 failed !")
                                      .arg(pluginsMetaData.at(idx).fileName()) << endl;
            return;
        }

        if (!plugin->versionCheck(QString::fromLatin1(kt::VERSION_STRING)))
        {
            Out(SYS_GEN | LOG_NOTICE) <<
                                      QStringLiteral("Plugin %1 version does not match KTorrent version, unloading it.")
                                      .arg(pluginsMetaData.at(idx).fileName()) << endl;

            delete plugin;
        }
        else
        {
            plugin->setCore(core);
            plugin->setGUI(gui);
            plugin->load();
            gui->mergePluginGui(plugin);
            plugin->loaded = true;
            loaded.insert(idx, plugin, true);
        }
    }

    void PluginManager::unload(const KPluginInfo& pi, int idx)
    {
        Q_UNUSED(pi)

        Plugin* p = loaded.find(idx);
        if (!p)
            return;

        // first shut it down properly
        bt::WaitJob* wjob = new WaitJob(2000);
        try
        {
            p->shutdown(wjob);
            if (wjob->needToWait())
                bt::WaitJob::execute(wjob);
            else
                delete wjob;
        }
        catch (Error& err)
        {
            Out(SYS_GEN | LOG_NOTICE) << "Error when unloading plugin: " << err.toString() << endl;
        }

        gui->removePluginGui(p);
        p->unload();
        p->loaded = false;
        loaded.erase(idx);
    }



    void PluginManager::unloadAll()
    {
        // first properly shutdown all plugins
        bt::WaitJob* wjob = new WaitJob(2000);
        try
        {
            bt::PtrMap<int, Plugin>::iterator i = loaded.begin();
            while (i != loaded.end())
            {
                Plugin* p = i->second;
                p->shutdown(wjob);
                i++;
            }
            if (wjob->needToWait())
                bt::WaitJob::execute(wjob);
            else
                delete wjob;
        }
        catch (Error& err)
        {
            Out(SYS_GEN | LOG_NOTICE) << "Error when unloading all plugins: " << err.toString() << endl;
        }

        // then unload them
        bt::PtrMap<int, Plugin>::iterator i = loaded.begin();
        while (i != loaded.end())
        {
            Plugin* p = i->second;
            gui->removePluginGui(p);
            p->unload();
            p->loaded = false;
            i++;
        }
        loaded.clear();
    }

    void PluginManager::updateGuiPlugins()
    {
        bt::PtrMap<int, Plugin>::iterator i = loaded.begin();
        while (i != loaded.end())
        {
            Plugin* p = i->second;
            p->guiUpdate();
            i++;
        }
    }

}
