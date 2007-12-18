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
#include <klocale.h>
#include <QNetworkInterface>
#include <solid/device.h>
#include <solid/networkinterface.h>
#include "networkpref.h"
#include "settings.h"

namespace kt
{

	NetworkPref::NetworkPref(QWidget* parent)
	: PrefPageInterface(Settings::self(),i18n("Network"),"preferences-system-network",parent)
	{
		setupUi(this);
	}


	NetworkPref::~NetworkPref()
	{
	}

	void NetworkPref::loadSettings()
	{
		kcfg_httpTrackerProxy->setEnabled(Settings::doNotUseKDEProxy());

		kcfg_networkInterface->addItem(KIcon("network-wired"),i18n("All interfaces"));

		// get all the network devices and add them to the combo box
		QList<QNetworkInterface> iface_list = QNetworkInterface::allInterfaces();
		
		QList<Solid::Device> netlist = Solid::Device::listFromType(Solid::DeviceInterface::NetworkInterface);
		
		
		foreach(QNetworkInterface iface,iface_list)
		{
			KIcon icon("network-wired");
			foreach (Solid::Device device,netlist)
			{
				Solid::NetworkInterface* netdev = device.as<Solid::NetworkInterface>();
				if (netdev->ifaceName() == iface.name() && netdev->isWireless())
				{
					icon = KIcon("network-wireless");
					break;
				}
					
			}
			
			kcfg_networkInterface->addItem(icon,iface.name());
		}
		
		kcfg_socksProxy->setEnabled(Settings::socksEnabled());
		kcfg_socksVersion->setEnabled(Settings::socksEnabled());
		kcfg_socksPort->setEnabled(Settings::socksEnabled());
		
		kcfg_socksPassword->setEnabled(Settings::socksUsePassword());
		kcfg_socksUsername->setEnabled(Settings::socksUsePassword());
	}

	void NetworkPref::loadDefaults()
	{
		kcfg_socksProxy->setEnabled(Settings::socksEnabled());
		kcfg_socksVersion->setEnabled(Settings::socksEnabled());
		kcfg_socksPort->setEnabled(Settings::socksEnabled());
		
		kcfg_socksPassword->setEnabled(Settings::socksUsePassword());
		kcfg_socksUsername->setEnabled(Settings::socksUsePassword());
	}
	
}
