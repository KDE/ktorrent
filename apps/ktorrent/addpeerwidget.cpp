/***************************************************************************
 *   Copyright (C) 2006 by Ivan Vasić   								   *
 *   ivasic@gmail.com   												   *
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
#include "addpeerwidget.h"

#include <util/constants.h>
#include <interfaces/torrentinterface.h>

#include <klocale.h>
#include <ksqueezedtextlabel.h>
#include <klineedit.h>
#include <knuminput.h>
#include <kmessagebox.h>

#include <qstring.h>
#include <qstringlist.h>
#include <qregexp.h>
#include <qvalidator.h>

using namespace kt;
using bt::Uint16;

//PeerSource

ManualPeerSource::ManualPeerSource()
{}

void ManualPeerSource::start()
{}

void ManualPeerSource::stop(bt::WaitJob* )
{}

ManualPeerSource::~ ManualPeerSource()
{}

void ManualPeerSource::signalPeersReady()
{
	peersReady(this);
}


//AddPeerWidget

AddPeerWidget::AddPeerWidget(kt::TorrentInterface* tc, QWidget *parent, const char *name)
	:AddPeerWidgetBase(parent, name), m_tc(tc)
{
	if(!tc)
	{
		//oops, something went wrong...
		m_status->setText(i18n("Torrent does not exist. Report this bug to KTorrent developers."));
		m_ip->setEnabled(false);
		m_port->setEnabled(false);
		return;
	}
	
	m_peerSource = new ManualPeerSource();
	
	//Register this peer source with ps manager.
	m_tc->addPeerSource(m_peerSource);
}

AddPeerWidget::~ AddPeerWidget()
{
	//Unregister this peer source with ps manager.
	m_tc->removePeerSource(m_peerSource);
	
	delete m_peerSource;
}

void AddPeerWidget::btnAdd_clicked()
{
	int var=0;
	
	QRegExp rx("[0-9]{1,3}(.[0-9]{1,3}){3,3}");
	QRegExpValidator v( rx,0);
	
	QString ip = m_ip->text();

	if(v.validate( ip, var ) == QValidator::Acceptable)
	{
		m_peerSource->addPeer(ip, m_port->value());
		
		m_peerSource->signalPeersReady();
		
		m_status->setText(i18n("Potential peer added."));
	}
	else
	{
		KMessageBox::sorry(0, i18n("Malformed IP address."));
		m_status->setText("");
	}
}

#include "addpeerwidget.moc"
