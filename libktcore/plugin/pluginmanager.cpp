/*
    SPDX-FileCopyrightText: 2005 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "pluginmanager.h"

#include <QFile>
#include <QTextStream>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KPluginMetaData>
#include <KSharedConfig>

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
    pluginsMetaData = KPluginMetaData::findPlugins(QStringLiteral("ktorrent_plugins"));

    if (!prefpage) {
        prefpage = new PluginActivity(this);
        gui->addActivity(prefpage);
    }

    prefpage->updatePluginList();
    loadPlugins();
    prefpage->update();
}

void PluginManager::loadPlugins()
{
    const KConfigGroup cfg = KSharedConfig::openConfig()->group(QStringLiteral("Plugins"));
    int idx = 0;
    for (const KPluginMetaData &data : std::as_const(pluginsMetaData)) {
        if (loaded.contains(idx) && !data.isEnabled(cfg)) {
            // unload it
            unload(data, idx);
        } else if (!loaded.contains(idx) && data.isEnabled(cfg)) {
            // load it
            load(data, idx);
        }
        idx++;
    }
}

void PluginManager::load(const KPluginMetaData &data, int idx)
{
    auto plugin = KPluginFactory::instantiatePlugin<kt::Plugin>(data).plugin;
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
