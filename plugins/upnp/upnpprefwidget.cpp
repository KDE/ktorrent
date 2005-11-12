/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/

#include <klistview.h>
#include <kpushbutton.h>
#include <torrent/udptrackersocket.h>
#include <torrent/globals.h>
#include <torrent/server.h>
#include "upnpprefwidget.h"
#include "upnppluginsettings.h"

using namespace bt;

namespace kt
{
	UPnPPrefWidget::UPnPPrefWidget(QWidget* parent, const char* name, WFlags fl)
			: UPnPWidget(parent,name,fl)
	{
		connect(m_forward_btn,SIGNAL(clicked()),this,SLOT(onForwardBtnClicked()));
		connect(m_undo_forward_btn,SIGNAL(clicked()),this,SLOT(onUndoForwardBtnClicked()));
		connect(m_rescan,SIGNAL(clicked()),this,SLOT(onRescanClicked()));
	}
	
	UPnPPrefWidget::~UPnPPrefWidget()
	{}
	
	
	void UPnPPrefWidget::addDevice(UPnPRouter* r)
	{
		connect(r,SIGNAL(replyError(const QString& )),this,SLOT(onReplyError(const QString& )));
		connect(r,SIGNAL(replyOK(const QString& )),this,SLOT(onReplyOK(const QString& )));
		KListViewItem* item = new KListViewItem(m_device_list,r->getDescription().friendlyName);
		itemmap[item] = r;
		// if we have discovered the default device or there is none
		// forward it's ports
		QString def_dev = UPnPPluginSettings::defaultDevice();
		if (def_dev == r->getServer() || def_dev == QString::null)
		{
			UPnPPluginSettings::setDefaultDevice(r->getServer());
			UPnPPluginSettings::writeConfig();
			
			// forward both ports
			r->forward(bt::Globals::instance().getServer().getPortInUse(),UPnPRouter::TCP);
			r->forward(bt::UDPTrackerSocket::getPort(),UPnPRouter::UDP);
		}
	}
		
	void UPnPPrefWidget::onForwardBtnClicked()
	{
		KListViewItem* item = (KListViewItem*)m_device_list->currentItem();;
		if (!item)
			return;
		
		UPnPRouter* r = itemmap[item];
		if (!r)
			return;
		
		r->forward(bt::Globals::instance().getServer().getPortInUse(),UPnPRouter::TCP);
		r->forward(bt::UDPTrackerSocket::getPort(),UPnPRouter::UDP);
		QString def_dev = UPnPPluginSettings::defaultDevice();
		if (def_dev != r->getServer())
		{
			UPnPPluginSettings::setDefaultDevice(r->getServer());
			UPnPPluginSettings::writeConfig();
		}
			
	}
	
	void UPnPPrefWidget::onRescanClicked()
	{
		// clear the list and emit the signal
		itemmap.clear();
		m_device_list->clear();
		rescan();
	}
	
	void UPnPPrefWidget::onUndoForwardBtnClicked()
	{
		KListViewItem* item = (KListViewItem*)m_device_list->currentItem();;
		if (!item)
			return;
		
		UPnPRouter* r =  itemmap[item];
		if (!r)
			return;
		
		r->undoForward(bt::Globals::instance().getServer().getPortInUse(),UPnPRouter::TCP);
		r->undoForward(bt::UDPTrackerSocket::getPort(),UPnPRouter::UDP);
		QString def_dev = UPnPPluginSettings::defaultDevice();
		if (def_dev == r->getServer())
		{
			UPnPPluginSettings::setDefaultDevice(QString::null);
			UPnPPluginSettings::writeConfig();
		}
	}
	
	void UPnPPrefWidget::onReplyOK(const QString &)
	{
		updatePortMappings();
	}
	
	void UPnPPrefWidget::updatePortMappings()
	{
		// update all port mappings
		QMap<KListViewItem*,UPnPRouter*>::iterator i = itemmap.begin();
		while (i != itemmap.end())
		{
			UPnPRouter* r = i.data();
			KListViewItem* item = i.key();
			QString msg;
			QValueList<UPnPRouter::Forwarding>::iterator j = r->beginPortMappings();
			while (j != r->endPortMappings())
			{
				UPnPRouter::Forwarding & f = *j;
				if (!f.pending)
				{
					msg += QString::number(f.port) + " (";
					QString prot = (f.prot == UPnPRouter::UDP ? "UDP" : "TCP");
					msg +=  prot + ") ";
				}
				j++;
			}
			item->setText(1,msg);
			i++;
		}
	}
	
	void UPnPPrefWidget::onReplyError(const QString & )
	{
		updatePortMappings();
	}
}



#include "upnpprefwidget.moc"

