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

#include <QHeaderView>

#include <KLocalizedString>
#include <KSharedConfig>
#include <KMessageBox>

#include <torrent/globals.h>
#include <util/log.h>
#include <util/error.h>
#include <upnp/upnprouter.h>
#include <upnp/upnpmcastsocket.h>
#include "upnpwidget.h"
#include "upnppluginsettings.h"
#include "routermodel.h"


using namespace bt;

namespace kt
{


    UPnPWidget::UPnPWidget(UPnPMCastSocket* sock, QWidget* parent)
        : QWidget(parent), sock(sock)
    {
        setupUi(this);
        m_devices->setRootIsDecorated(false);
        connect(m_forward, &QPushButton::clicked, this, &UPnPWidget::onForwardBtnClicked);
        connect(m_undo_forward, &QPushButton::clicked, this, &UPnPWidget::onUndoForwardBtnClicked);
        connect(m_rescan, &QPushButton::clicked, this, &UPnPWidget::onRescanClicked);
        connect(sock, &bt::UPnPMCastSocket::discovered, this, &UPnPWidget::addDevice);

        bt::Globals::instance().getPortList().setListener(this);

        model = new RouterModel(this);
        m_devices->setModel(model);

        // load the state of the devices treewidget
        KConfigGroup g = KSharedConfig::openConfig()->group("UPnPDevicesList");
        QByteArray s = QByteArray::fromBase64(g.readEntry("state", QByteArray()));
        if (!s.isEmpty())
            m_devices->header()->restoreState(s);

        m_forward->setEnabled(false);
        m_undo_forward->setEnabled(false);
        connect(m_devices->selectionModel(), SIGNAL(currentChanged(const QModelIndex& , const QModelIndex&)),
                this, SLOT(onCurrentDeviceChanged(const QModelIndex&, const QModelIndex&)));
    }


    UPnPWidget::~UPnPWidget()
    {
        bt::Globals::instance().getPortList().setListener(0);
    }

    void UPnPWidget::shutdown(bt::WaitJob* job)
    {
        // save the state of the devices treewidget
        KConfigGroup g = KSharedConfig::openConfig()->group("UPnPDevicesList");
        QByteArray s = m_devices->header()->saveState();
        g.writeEntry("state", s.toBase64());

        net::PortList& pl = bt::Globals::instance().getPortList();
        for (net::PortList::iterator i = pl.begin(); i != pl.end(); i++)
            model->undoForward(*i, job);
    }

    void UPnPWidget::addDevice(bt::UPnPRouter* r)
    {
        connect(r, SIGNAL(stateChanged()), this, SLOT(updatePortMappings()));
        model->addRouter(r);

        Out(SYS_PNP | LOG_DEBUG) << "Doing port mappings for " << r->getServer() << endl;
        try
        {
            net::PortList& pl = bt::Globals::instance().getPortList();

            for (net::PortList::iterator i = pl.begin(); i != pl.end(); i++)
            {
                if (i->forward)
                    r->forward(*i);
            }
        }
        catch (Error& e)
        {
            KMessageBox::error(this, e.toString());
        }
    }

    void UPnPWidget::onForwardBtnClicked()
    {
        UPnPRouter* r = model->routerForIndex(m_devices->selectionModel()->currentIndex());
        if (!r)
            return;

        try
        {
            net::PortList& pl = bt::Globals::instance().getPortList();

            for (net::PortList::iterator i = pl.begin(); i != pl.end(); i++)
            {
                net::Port& p = *i;
                if (p.forward)
                    r->forward(p);
            }
        }
        catch (Error& e)
        {
            KMessageBox::error(this, e.toString());
        }
    }

    void UPnPWidget::onUndoForwardBtnClicked()
    {
        UPnPRouter* r = model->routerForIndex(m_devices->selectionModel()->currentIndex());
        if (!r)
            return;

        try
        {
            net::PortList& pl = bt::Globals::instance().getPortList();

            for (net::PortList::iterator i = pl.begin(); i != pl.end(); i++)
            {
                net::Port& p = *i;
                if (p.forward)
                    r->undoForward(p);
            }
        }
        catch (Error& e)
        {
            KMessageBox::error(this, e.toString());
        }
    }

    void UPnPWidget::onRescanClicked()
    {
        sock->discover();
    }

    void UPnPWidget::updatePortMappings()
    {
        // update all port mappings
        model->update();
        onCurrentDeviceChanged(m_devices->selectionModel()->currentIndex(), QModelIndex());
    }

    void UPnPWidget::portAdded(const net::Port& port)
    {
        model->forward(port);
    }

    void UPnPWidget::portRemoved(const net::Port& port)
    {
        model->undoForward(port, nullptr);
    }

    void UPnPWidget::onCurrentDeviceChanged(const QModelIndex& current, const QModelIndex& previous)
    {
        Q_UNUSED(previous);
        UPnPRouter* r = model->routerForIndex(current);
        m_forward->setEnabled(r != nullptr);
        m_undo_forward->setEnabled(r != nullptr && model->rowCount(QModelIndex()) > 0);
    }

}
