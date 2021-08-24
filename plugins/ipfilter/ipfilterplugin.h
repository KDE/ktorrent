/*
    SPDX-FileCopyrightText: 2005 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTIPFILTERPLUGIN_H
#define KTIPFILTERPLUGIN_H

#include "ipblockingprefpage.h"
#include "ipblocklist.h"
#include <QTimer>
#include <interfaces/plugin.h>

class QString;

namespace kt
{
class IPBlockingPrefPage;

const int AUTO_UPDATE_RETRY_INTERVAL = 15 * 60; // seconds

/**
 * @author Ivan Vasic <ivasic@gmail.com>
 * @brief IP filter plugin
 *
 * This plugin will load IP ranges from specific files into KT IPBlocklist.
 */
class IPFilterPlugin : public Plugin
{
    Q_OBJECT
public:
    IPFilterPlugin(QObject *parent, const KPluginMetaData &data, const QVariantList &args);
    ~IPFilterPlugin() override;

    void load() override;
    void unload() override;

    /// Loads the KT format list filter
    void loadFilters();

    /// Loads the anti-p2p filter list
    bool loadAntiP2P();

    /// Unloads the anti-p2p filter list
    bool unloadAntiP2P();

    /// Whether or not the IP filter is loaded and running
    bool loadedAndRunning();

public Q_SLOTS:
    void checkAutoUpdate();
    void notification(const QString &msg);

private:
    IPBlockingPrefPage *pref;
    QScopedPointer<IPBlockList> ip_filter;
    QTimer auto_update_timer;
};

}

#endif
