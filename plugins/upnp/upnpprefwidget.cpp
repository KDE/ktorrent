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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/

#include <klistview.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <torrent/udptrackersocket.h>
#include <torrent/globals.h>
#include <torrent/server.h>
#include <kademlia/dhtbase.h>
#include "upnpprefwidget.h"
#include <util/log.h>
#include <util/error.h>
#include <torrent/globals.h>
#include "upnppluginsettings.h"

using namespace bt;

namespace kt
{
	UPnPPrefWidget::UPnPPrefWidget(QWidget* parent, const char* name, WFlags fl)
			: UPnPWidget(parent,name,fl)
	{
		def_router = 0;
		connect(m_forward_btn,SIGNAL(clicked()),this,SLOT(onForwardBtnClicked()));
		connect(m_undo_forward_btn,SIGNAL(clicked()),this,SLOT(onUndoForwardBtnClicked()));
		connect(m_rescan,SIGNAL(clicked()),this,SLOT(onRescanClicked()));
	}
	
	UPnPPrefWidget::~UPnPPrefWidget()
	{
		if (def_router)
		{
			try
			{
				def_router->undoForward(bt::Globals::instance().getServer().getPortInUse(),UPnPRouter::TCP);
				def_router->undoForward(bt::UDPTrackerSocket::getPort(),UPnPRouter::UDP);
				def_router->undoForward(bt::Globals::instance().getDHT().getPort(),UPnPRouter::UDP);
			}
			catch (Error & e)
			{
				Out(SYS_PNP|LOG_DEBUG) << "Error : " << e.toString() << endl;
			}
		}
	}
	
	
	void UPnPPrefWidget::addDevice(UPnPRouter* r)
	{
		connect(r,SIGNAL(updateGUI()),this,SLOT(updatePortMappings()));
		KListViewItem* item = new KListViewItem(m_device_list,r->getDescription().friendlyName);
		itemmap[item] = r;
		// if we have discovered the default device or there is none
		// forward it's ports
		QString def_dev = UPnPPluginSettings::defaultDevice();
		if (def_dev == r->getServer() || def_dev.length() == 0)
		{
			Out(SYS_PNP|LOG_DEBUG) << "Doing default port mappings ..." << endl;
			UPnPPluginSettings::setDefaultDevice(r->getServer());
			UPnPPluginSettings::writeConfig();
			
			try
			{
				// forward both ports
				r->forward(bt::Globals::instance().getServer().getPortInUse(),UPnPRouter::TCP);
				r->forward(bt::UDPTrackerSocket::getPort(),UPnPRouter::UDP);
				if (bt::Globals::instance().getDHT().isRunning())
					r->forward(bt::Globals::instance().getDHT().getPort(),UPnPRouter::UDP);
				def_router = r;
			}
			catch (Error & e)
			{
				KMessageBox::error(this,e.toString());
			}
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
		
		try
		{
			r->forward(bt::Globals::instance().getServer().getPortInUse(),UPnPRouter::TCP);
			r->forward(bt::UDPTrackerSocket::getPort(),UPnPRouter::UDP);
			if (bt::Globals::instance().getDHT().isRunning())
				r->forward(bt::Globals::instance().getDHT().getPort(),UPnPRouter::UDP);
			QString def_dev = UPnPPluginSettings::defaultDevice();
			if (def_dev != r->getServer())
			{
				UPnPPluginSettings::setDefaultDevice(r->getServer());
				UPnPPluginSettings::writeConfig();
				def_router = r;
			}
			
		}
		catch (Error & e)
		{
			KMessageBox::error(this,e.toString());
		}	
	}
	
	void UPnPPrefWidget::onRescanClicked()
	{
		// clear the list and emit the signal
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
		
		try
		{
			r->undoForward(bt::Globals::instance().getServer().getPortInUse(),UPnPRouter::TCP);
			r->undoForward(bt::UDPTrackerSocket::getPort(),UPnPRouter::UDP);
			if (bt::Globals::instance().getDHT().isRunning())
				r->undoForward(bt::Globals::instance().getDHT().getPort(),UPnPRouter::UDP);
			
			QString def_dev = UPnPPluginSettings::defaultDevice();
			if (def_dev == r->getServer())
			{
				UPnPPluginSettings::setDefaultDevice(QString::null);
				UPnPPluginSettings::writeConfig();
				def_router = 0;
			}
		}
		catch (Error & e)
		{
			KMessageBox::error(this,e.toString());
		}
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
	

}



#include "upnpprefwidget.moc"

