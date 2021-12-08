/*
    SPDX-FileCopyrightText: 2005 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTPLUGINMANAGER_H
#define KTPLUGINMANAGER_H

#include <QList>
#include <QStringList>
#include <QVector>

#include <KPluginInfo>
#include <KPluginMetaData>
#include <KSharedConfig>

#include <interfaces/plugin.h>
#include <ktcore_export.h>
#include <util/ptrmap.h>

namespace kt
{
class CoreInterface;
class GUIInterface;
class PluginActivity;

/**
 * @author Joris Guisson
 * @brief Class to manage plugins
 *
 * This class manages all plugins. Plugins are stored in a map
 */
class KTCORE_EXPORT PluginManager
{
    QVector<KPluginMetaData> pluginsMetaData;
    CoreInterface *core;
    GUIInterface *gui;
    PluginActivity *prefpage;
    bt::PtrMap<int, Plugin> loaded;

public:
    PluginManager(CoreInterface *core, GUIInterface *gui);
    ~PluginManager();

    QVector<KPluginMetaData> pluginsMetaDataList() const
    {
        return pluginsMetaData;
    }

    /**
     * Load the list of plugins.
     * This basically uses KTrader to get a list of available plugins, and
     * loads those, but does not initialize them. We will consider a plugin loaded
     * when it's load method is called.
     */
    void loadPluginList();

    /**
     * Check the PluginInfo of each plugin and unload or load it if necessary
     */
    void loadPlugins();

    /**
     * Update all plugins who need a periodical GUI update.
     */
    void updateGuiPlugins();

    /**
     * Unload all plugins.
     */
    void unloadAll();

private:
    void load(const KPluginMetaData &data, int idx);
    void unload(const KPluginMetaData &data, int idx);
};

}

#endif
