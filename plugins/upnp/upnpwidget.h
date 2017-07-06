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

#ifndef KTUPNPPREFPAGE_H
#define KTUPNPPREFPAGE_H

#include <QWidget>
#include <net/portlist.h>
#include "ui_upnpwidget.h"


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
        UPnPWidget(bt::UPnPMCastSocket* sock, QWidget* parent);
        ~UPnPWidget();

        void shutdown(bt::WaitJob* job);

    public slots:
        /**
         * Add a device to the list.
         * @param r The device
         */
        void addDevice(bt::UPnPRouter* r);

    protected slots:
        void onForwardBtnClicked();
        void onUndoForwardBtnClicked();
        void onRescanClicked();
        void updatePortMappings();
        void onCurrentDeviceChanged(const QModelIndex& current, const QModelIndex& previous);

    private:
        void portAdded(const net::Port& port) override;
        void portRemoved(const net::Port& port) override;

    private:
        bt::UPnPMCastSocket* sock;
        RouterModel* model;
    };
}

#endif
