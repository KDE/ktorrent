/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
#include <util/log.h>
#include "upnpmcastsocket.h"
#include "upnptestwidget.h"
#include "upnprouter.h"

using namespace bt;
using namespace kt;

UPnPTestWidget::UPnPTestWidget(QWidget* parent) : QWidget(parent)
{
	setupUi(this);
	connect(m_find_routers,SIGNAL(clicked()),this,SLOT(findRouters()));
	connect(m_forward,SIGNAL(clicked()),this,SLOT(doForward()));
	connect(m_undo_forward,SIGNAL(clicked()),this,SLOT(undoForward()));
	mcast_socket = 0;
	router = 0;

	m_forward->setEnabled(false);
	m_undo_forward->setEnabled(false);
	m_port->setEnabled(false);
	m_protocol->setEnabled(false);

	AddLogMonitor(this);
}

UPnPTestWidget::~UPnPTestWidget()
{
	if (mcast_socket)
		delete mcast_socket;
}

void UPnPTestWidget::doForward()
{
	QString proto = m_protocol->currentText();
	bt::Uint16 port = m_port->value();
	Out(SYS_GEN|LOG_DEBUG) << "Forwarding port " << port << " (" << proto << ")" << endl;
	net::Port p(port,proto == "UDP" ? net::UDP : net::TCP,true);
	router->forward(p);
}
	
void UPnPTestWidget::undoForward()
{
	QString proto = m_protocol->currentText();
	bt::Uint16 port = m_port->value();
	Out(SYS_GEN|LOG_DEBUG) << "Unforwarding port " << port << " (" << proto << ")" << endl;
	net::Port p(port,proto == "UDP" ? net::UDP : net::TCP,true);
	router->undoForward(p);
}

void UPnPTestWidget::findRouters()
{
	Out(SYS_GEN|LOG_DEBUG) << "Searching for routers ..." << endl;
	if (!mcast_socket)
	{
		mcast_socket = new UPnPMCastSocket();
		connect(mcast_socket,SIGNAL(discovered(kt::UPnPRouter*)),this,SLOT(discovered(kt::UPnPRouter*)));
	}

	mcast_socket->discover();
}

void UPnPTestWidget::discovered(kt::UPnPRouter* r)
{
	router = r;
	m_router->setText(router->getServer());
	m_forward->setEnabled(true);
	m_undo_forward->setEnabled(true);
	m_port->setEnabled(true);
	m_protocol->setEnabled(true);
	router->setVerbose(true);
}

void UPnPTestWidget::message(const QString & line, unsigned int arg)
{
	m_text_output->append(line);
}

#include "upnptestwidget.moc"
