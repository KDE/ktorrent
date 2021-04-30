/*
    SPDX-FileCopyrightText: 2005-2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTUPNPPREFPAGE_H
#define KTUPNPPREFPAGE_H

#include "ui_upnpwidget.h"
#include <QWidget>
#include <net/portlist.h>

namespace bt
{
class WaitJob;
class UPnPRouter;
class UPnPMCastSocket;
}

namespace kt
{
class RouterModel;

/**
 * @author Joris Guisson
 *
 * Page in the preference dialog for the UPnP plugin.
 */
class UPnPWidget : public QWidget, public Ui_UPnPWidget, public net::PortListener
{
    Q_OBJECT
public:
    UPnPWidget(bt::UPnPMCastSocket *sock, QWidget *parent);
    ~UPnPWidget() override;

    void shutdown(bt::WaitJob *job);

public Q_SLOTS:
    /**
     * Add a device to the list.
     * @param r The device
     */
    void addDevice(bt::UPnPRouter *r);

protected Q_SLOTS:
    void onForwardBtnClicked();
    void onUndoForwardBtnClicked();
    void onRescanClicked();
    void updatePortMappings();
    void onCurrentDeviceChanged(const QModelIndex &current, const QModelIndex &previous);

private:
    void portAdded(const net::Port &port) override;
    void portRemoved(const net::Port &port) override;

private:
    bt::UPnPMCastSocket *sock;
    RouterModel *model;
};
}

#endif
