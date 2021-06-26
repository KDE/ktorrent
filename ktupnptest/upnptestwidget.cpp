/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "upnptestwidget.h"
#include <upnp/upnpmcastsocket.h>
#include <upnp/upnprouter.h>
#include <util/log.h>

using namespace bt;

UPnPTestWidget::UPnPTestWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    connect(m_find_routers, &QPushButton::clicked, this, &UPnPTestWidget::findRouters);
    connect(m_forward, &QPushButton::clicked, this, &UPnPTestWidget::doForward);
    connect(m_undo_forward, &QPushButton::clicked, this, &UPnPTestWidget::undoForward);
    connect(m_verbose, &QCheckBox::toggled, this, &UPnPTestWidget::verboseModeChecked);
    mcast_socket = nullptr;
    router = nullptr;

    m_forward->setEnabled(false);
    m_undo_forward->setEnabled(false);
    m_port->setEnabled(false);
    m_protocol->setEnabled(false);

    AddLogMonitor(this);
}

UPnPTestWidget::~UPnPTestWidget()
{
    delete mcast_socket;
}

void UPnPTestWidget::doForward()
{
    QString proto = m_protocol->currentText();
    bt::Uint16 port = m_port->value();
    Out(SYS_GEN | LOG_DEBUG) << "Forwarding port " << port << " (" << proto << ")" << endl;
    net::Port p(port, proto == QStringLiteral("UDP") ? net::UDP : net::TCP, true);
    router->forward(p);
}

void UPnPTestWidget::undoForward()
{
    QString proto = m_protocol->currentText();
    bt::Uint16 port = m_port->value();
    Out(SYS_GEN | LOG_DEBUG) << "Unforwarding port " << port << " (" << proto << ")" << endl;
    net::Port p(port, proto == QStringLiteral("UDP") ? net::UDP : net::TCP, true);
    router->undoForward(p);
}

void UPnPTestWidget::findRouters()
{
    Out(SYS_GEN | LOG_DEBUG) << "Searching for routers ..." << endl;
    if (!mcast_socket) {
        mcast_socket = new UPnPMCastSocket(m_verbose->isChecked());
        connect(mcast_socket, &UPnPMCastSocket::discovered, this, &UPnPTestWidget::discovered);
    }

    mcast_socket->discover();
}

void UPnPTestWidget::discovered(bt::UPnPRouter *r)
{
    router = r;
    m_router->setText(router->getServer());
    m_forward->setEnabled(true);
    m_undo_forward->setEnabled(true);
    m_port->setEnabled(true);
    m_protocol->setEnabled(true);
    router->setVerbose(true);
}

void UPnPTestWidget::message(const QString &line, unsigned int arg)
{
    Q_UNUSED(arg);
    m_text_output->append(line);
}

void UPnPTestWidget::verboseModeChecked(bool on)
{
    if (mcast_socket)
        mcast_socket->setVerbose(on);
}
