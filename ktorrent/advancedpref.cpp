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
#include <solid/device.h>
#include <solid/networkinterface.h>
#include "advancedpref.h"
#include "settings.h"

namespace kt
{
	AdvancedPref::AdvancedPref(QWidget* parent) : PrefPageInterface(Settings::self(),i18n("Advanced"),"configure",parent)
	{
		setupUi(this);
		connect(kcfg_doUploadDataCheck,SIGNAL(toggled(bool)),this,SLOT(onUploadDataCheckToggled(bool)));
	}

	AdvancedPref::~AdvancedPref()
	{
	}

	void AdvancedPref::loadSettings()
	{
		kcfg_httpTrackerProxy->setEnabled(Settings::doNotUseKDEProxy());

		kcfg_networkInterface->addItem(KIcon("network-wired"),i18n("All interfaces"));

		// get all the network devices and add them to the combo box
		QList<Solid::Device> netlist = Solid::Device::listFromType(Solid::DeviceInterface::NetworkInterface);
		foreach (Solid::Device device,netlist)
		{
			Solid::NetworkInterface* netdev = device.as<Solid::NetworkInterface>();
			if (netdev)
				kcfg_networkInterface->addItem(
						KIcon(netdev->isWireless() ? "network-wireless" : "network-wired"),
						netdev->ifaceName());
		}

		kcfg_maxCorruptedBeforeRecheck->setEnabled(Settings::autoRecheck());
		kcfg_useMaxSizeForUploadDataCheck->setEnabled(Settings::doUploadDataCheck());
		kcfg_maxSizeForUploadDataCheck->setEnabled(Settings::doUploadDataCheck() && Settings::useMaxSizeForUploadDataCheck());
	}

	void AdvancedPref::loadDefaults()
	{
		loadSettings();
	}

	void AdvancedPref::onUploadDataCheckToggled(bool on)
	{
		if (!on)
		{
			kcfg_useMaxSizeForUploadDataCheck->setEnabled(false);
			kcfg_maxSizeForUploadDataCheck->setEnabled(false);
		}
		else
		{
			kcfg_useMaxSizeForUploadDataCheck->setEnabled(true);
			kcfg_maxSizeForUploadDataCheck->setEnabled(kcfg_useMaxSizeForUploadDataCheck->isChecked());
		}
	}
}

#include "advancedpref.moc"
