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
#include <klocalizedstring.h>
#include <QNetworkInterface>
#include <solid/device.h>
#include <util/log.h>
#include "networkpref.h"
#include "settings.h"

using namespace bt;

namespace kt
{

    NetworkPref::NetworkPref(QWidget* parent)
        : PrefPageInterface(Settings::self(), i18n("Network"), "preferences-system-network", parent)
    {
        setupUi(this);
        connect(m_recommended_settings, SIGNAL(clicked()), this, SIGNAL(calculateRecommendedSettings()));
        connect(kcfg_utpEnabled, SIGNAL(toggled(bool)), this, SLOT(utpEnabled(bool)));
        connect(kcfg_onlyUseUtp, SIGNAL(toggled(bool)), this, SLOT(onlyUseUtpEnabled(bool)));
    }


    NetworkPref::~NetworkPref()
    {
    }


    void NetworkPref::loadSettings()
    {
        kcfg_maxDownloadRate->setValue(Settings::maxDownloadRate());
        kcfg_maxUploadRate->setValue(Settings::maxUploadRate());
        kcfg_maxConnections->setValue(Settings::maxConnections());
        kcfg_maxTotalConnections->setValue(Settings::maxTotalConnections());

        combo_networkInterface->clear();
        combo_networkInterface->addItem(QIcon::fromTheme("network-wired"), i18n("All interfaces"));

        kcfg_onlyUseUtp->setEnabled(Settings::utpEnabled());
        kcfg_primaryTransportProtocol->setEnabled(Settings::utpEnabled() && !Settings::onlyUseUtp());

        // get all the network devices and add them to the combo box
        QList<QNetworkInterface> iface_list = QNetworkInterface::allInterfaces();

        // KF5 QList<Solid::Device> netlist = Solid::Device::listFromType(Solid::DeviceInterface::NetworkInterface);


        foreach (const QNetworkInterface& iface, iface_list)
        {
            QIcon icon = QIcon::fromTheme("network-wired");
#if 0 //KF5
            foreach (const Solid::Device& device, netlist)
            {
                const Solid::NetworkInterface* netdev = device.as<Solid::NetworkInterface>();
                if (netdev->ifaceName() == iface.name() && netdev->isWireless())
                {
                    icon = QIcon::fromTheme("network-wireless");
                    break;
                }

            }
#endif

            combo_networkInterface->addItem(icon, iface.name());
        }
        QString iface = Settings::networkInterface();
        int idx = (iface.isEmpty())? 0 /*all*/: combo_networkInterface->findText(iface);
        if (idx < 0)
        {
            bool ok;
            iface.toInt(&ok);
            if (ok)
            {
                idx = 0;
            }
            else
            {
                combo_networkInterface->addItem(iface);
                idx = combo_networkInterface->findText(iface);
            }
        }
        combo_networkInterface->setCurrentIndex(idx);
    }

    void NetworkPref::updateSettings()
    {
        QString iface = combo_networkInterface->currentText();
        Settings::setNetworkInterface(iface);
    }

    void NetworkPref::loadDefaults()
    {
    }


    void NetworkPref::utpEnabled(bool on)
    {
        kcfg_onlyUseUtp->setEnabled(on);
        kcfg_primaryTransportProtocol->setEnabled(on && !kcfg_onlyUseUtp->isChecked());
    }

    void NetworkPref::onlyUseUtpEnabled(bool on)
    {
        kcfg_primaryTransportProtocol->setEnabled(!on && kcfg_utpEnabled->isChecked());
    }


}
