/*
    SPDX-FileCopyrightText: 2005-2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTSEARCHPLUGIN_H
#define KTSEARCHPLUGIN_H

#include <interfaces/plugin.h>

namespace bt
{
class UPnPMCastSocket;
}

namespace kt
{
class UPnPWidget;

/**
@author Joris Guisson
*/
class UPnPPlugin : public Plugin
{
    Q_OBJECT
public:
    UPnPPlugin(QObject *parent, const QVariantList &args);
    ~UPnPPlugin() override;

    void load() override;
    void unload() override;
    void shutdown(bt::WaitJob *job) override;
    bool versionCheck(const QString &version) const override;

private:
    bt::UPnPMCastSocket *sock;
    UPnPWidget *upnp_tab;
};

}

#endif
