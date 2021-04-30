/*
    SPDX-FileCopyrightText: 2005-2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QHeaderView>

#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>

#include "routermodel.h"
#include "upnppluginsettings.h"
#include "upnpwidget.h"
#include <torrent/globals.h>
#include <upnp/upnpmcastsocket.h>
#include <upnp/upnprouter.h>
#include <util/error.h>
#include <util/log.h>

using namespace bt;

namespace kt
{
UPnPWidget::UPnPWidget(UPnPMCastSocket *sock, QWidget *parent)
    : QWidget(parent)
    , sock(sock)
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
    connect(m_devices->selectionModel(), &QItemSelectionModel::currentChanged, this, &UPnPWidget::onCurrentDeviceChanged);
}

UPnPWidget::~UPnPWidget()
{
    bt::Globals::instance().getPortList().setListener(0);
}

void UPnPWidget::shutdown(bt::WaitJob *job)
{
    // save the state of the devices treewidget
    KConfigGroup g = KSharedConfig::openConfig()->group("UPnPDevicesList");
    QByteArray s = m_devices->header()->saveState();
    g.writeEntry("state", s.toBase64());

    const net::PortList &pl = bt::Globals::instance().getPortList();
    for (const net::Port &p : pl)
        model->undoForward(p, job);
}

void UPnPWidget::addDevice(bt::UPnPRouter *r)
{
    connect(r, &UPnPRouter::stateChanged, this, &UPnPWidget::updatePortMappings);
    model->addRouter(r);

    Out(SYS_PNP | LOG_DEBUG) << "Doing port mappings for " << r->getServer() << endl;
    try {
        const net::PortList &pl = bt::Globals::instance().getPortList();

        for (const net::Port &p : pl) {
            if (p.forward)
                r->forward(p);
        }
    } catch (Error &e) {
        KMessageBox::error(this, e.toString());
    }
}

void UPnPWidget::onForwardBtnClicked()
{
    UPnPRouter *r = model->routerForIndex(m_devices->selectionModel()->currentIndex());
    if (!r)
        return;

    try {
        const net::PortList &pl = bt::Globals::instance().getPortList();

        for (const net::Port &p : pl) {
            if (p.forward)
                r->forward(p);
        }
    } catch (Error &e) {
        KMessageBox::error(this, e.toString());
    }
}

void UPnPWidget::onUndoForwardBtnClicked()
{
    UPnPRouter *r = model->routerForIndex(m_devices->selectionModel()->currentIndex());
    if (!r)
        return;

    try {
        const net::PortList &pl = bt::Globals::instance().getPortList();

        for (const net::Port &p : pl) {
            if (p.forward)
                r->undoForward(p);
        }
    } catch (Error &e) {
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

void UPnPWidget::portAdded(const net::Port &port)
{
    model->forward(port);
}

void UPnPWidget::portRemoved(const net::Port &port)
{
    model->undoForward(port, nullptr);
}

void UPnPWidget::onCurrentDeviceChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    UPnPRouter *r = model->routerForIndex(current);
    m_forward->setEnabled(r != nullptr);
    m_undo_forward->setEnabled(r != nullptr && model->rowCount(QModelIndex()) > 0);
}

}
