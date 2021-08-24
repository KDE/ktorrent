/*
    SPDX-FileCopyrightText: 2005-2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTZEROCONFPLUGIN_H
#define KTZEROCONFPLUGIN_H

#include <interfaces/coreinterface.h>
#include <interfaces/plugin.h>
#include <interfaces/torrentinterface.h>
#include <util/ptrmap.h>

namespace kt
{
class TorrentService;

/**
 * @author Joris Guisson <joris.guisson@gmail.com>
 *
 * Plugin which handles the zeroconf service.
 */
class ZeroConfPlugin : public Plugin
{
    Q_OBJECT
public:
    ZeroConfPlugin(QObject *parent, const QVariantList &args);
    ~ZeroConfPlugin() override;

    void load() override;
    void unload() override;

private Q_SLOTS:
    /**
     * A TorrentInterface was added
     * @param tc
     */
    void torrentAdded(bt::TorrentInterface *tc);

    /**
     * A TorrentInterface was removed
     * @param tc
     */
    void torrentRemoved(bt::TorrentInterface *tc);

    /**
     * An AvahiService has been destroyed by the psman
     */
    void avahiServiceDestroyed(TorrentService *av);

private:
    bt::PtrMap<bt::TorrentInterface *, TorrentService> services;
};

}

#endif
