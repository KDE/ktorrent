/*
    SPDX-FileCopyrightText: 2005 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "pluginmanager.h"

#include <QFile>
#include <QTextStream>

#include <KLocalizedString>
#include <KPluginMetaData>

#include "pluginactivity.h"
#include <interfaces/guiinterface.h>
#include <torrent/globals.h>
#include <util/error.h>
#include <util/fileops.h>
#include <util/log.h>
#include <util/waitjob.h>

using namespace bt;

namespace kt
{
PluginManager::PluginManager(CoreInterface *core, GUIInterface *gui)
    : core(core)
    , gui(gui)
{
    prefpage = nullptr;
    loaded.setAutoDelete(true);
}

PluginManager::~PluginManager()
{
    delete prefpage;
}

void PluginManager::loadPluginList()
{
    pluginsMetaData = KPluginLoader::findPlugins(QStringLiteral("ktorrent"));

    if (!prefpage) {
        prefpage = new PluginActivity(this);
        gui->addActivity(prefpage);
    }

    prefpage->updatePluginList();
    loadPlugins();
    prefpage->update();
}

inline bool isPluginEnabled(const KPluginMetaData &data)
{
    return KSharedConfig::openConfig()->group(data.pluginId()).readEntry(data.pluginId() + QLatin1String("Enabled"), data.isEnabledByDefault());
}

void PluginManager::loadPlugins()
{
    int idx = 0;
    for (const KPluginMetaData &data : qAsConst(pluginsMetaData)) {
        if (loaded.contains(idx) && !isPluginEnabled(data)) {
            // unload it
            unload(data, idx);
        } else if (!loaded.contains(idx) && isPluginEnabled(data)) {
            // load it
            load(data, idx);
        }
        idx++;
    }
}

void PluginManager::load(const KPluginMetaData &data, int idx)
{
    Q_UNUSED(data)
    KPluginLoader loader(data.fileName());
    KPluginFactory *factory = loader.factory();
    if (!factory)
        return;

    Plugin *plugin = factory->create<kt::Plugin>();
    if (!plugin) {
        Out(SYS_GEN | LOG_NOTICE) << QStringLiteral("Creating instance of plugin %1 failed !").arg(pluginsMetaData.at(idx).fileName()) << endl;
        return;
    }

    plugin->setCore(core);
    plugin->setGUI(gui);
    plugin->load();
    gui->mergePluginGui(plugin);
    plugin->setIsLoaded(true);
    loaded.insert(idx, plugin, true);
}

void PluginManager::unload(const KPluginMetaData &data, int idx)
{
    Q_UNUSED(data)

    Plugin *p = loaded.find(idx);
    if (!p)
        return;

    // first shut it down properly
    bt::WaitJob *wjob = new WaitJob(2000);
    try {
        p->shutdown(wjob);
        if (wjob->needToWait())
            bt::WaitJob::execute(wjob);
        else
            delete wjob;
    } catch (Error &err) {
        Out(SYS_GEN | LOG_NOTICE) << "Error when unloading plugin: " << err.toString() << endl;
    }

    gui->removePluginGui(p);
    p->unload();
    p->setIsLoaded(false);
    loaded.erase(idx);
}

void PluginManager::unloadAll()
{
    // first properly shutdown all plugins
    bt::WaitJob *wjob = new WaitJob(2000);
    try {
        bt::PtrMap<int, Plugin>::iterator i = loaded.begin();
        while (i != loaded.end()) {
            Plugin *p = i->second;
            p->shutdown(wjob);
            i++;
        }
        if (wjob->needToWait())
            bt::WaitJob::execute(wjob);
        else
            delete wjob;
    } catch (Error &err) {
        Out(SYS_GEN | LOG_NOTICE) << "Error when unloading all plugins: " << err.toString() << endl;
    }

    // then unload them
    bt::PtrMap<int, Plugin>::iterator i = loaded.begin();
    while (i != loaded.end()) {
        Plugin *p = i->second;
        gui->removePluginGui(p);
        p->unload();
        p->setIsLoaded(false);
        i++;
    }
    loaded.clear();
}

void PluginManager::updateGuiPlugins()
{
    bt::PtrMap<int, Plugin>::iterator i = loaded.begin();
    while (i != loaded.end()) {
        Plugin *p = i->second;
        p->guiUpdate();
        i++;
    }
}

}
