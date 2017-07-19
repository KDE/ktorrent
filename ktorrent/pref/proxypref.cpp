/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
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

#include "proxypref.h"
#include "settings.h"

namespace kt
{

    ProxyPref::ProxyPref(QWidget* parent) : PrefPageInterface(Settings::self(), i18n("Proxy"), QStringLiteral("preferences-system-network"), parent)
    {
        setupUi(this);
        connect(kcfg_socksEnabled, &QCheckBox::toggled, this, &ProxyPref::socksEnabledToggled);
        connect(kcfg_socksUsePassword, &QCheckBox::toggled, this, &ProxyPref::usernamePasswordToggled);
    }


    ProxyPref::~ProxyPref()
    {
    }


    void ProxyPref::loadDefaults()
    {
        loadSettings();
    }

    void ProxyPref::loadSettings()
    {
        kcfg_httpProxy->setEnabled(!Settings::useKDEProxySettings());
        kcfg_httpProxyPort->setEnabled(!Settings::useKDEProxySettings());
        kcfg_useProxyForWebSeeds->setEnabled(!Settings::useKDEProxySettings());
        kcfg_useProxyForTracker->setEnabled(!Settings::useKDEProxySettings());

        kcfg_socksProxy->setEnabled(Settings::socksEnabled());
        kcfg_socksVersion->setEnabled(Settings::socksEnabled());
        kcfg_socksPort->setEnabled(Settings::socksEnabled());

        kcfg_socksUsePassword->setEnabled(Settings::socksEnabled());
        kcfg_socksPassword->setEnabled(Settings::socksUsePassword() && Settings::socksEnabled());
        kcfg_socksUsername->setEnabled(Settings::socksUsePassword() && Settings::socksEnabled());
    }

    void ProxyPref::updateSettings()
    {
    }

    void ProxyPref::socksEnabledToggled(bool on)
    {
        kcfg_socksUsePassword->setEnabled(on);
        kcfg_socksPassword->setEnabled(on && kcfg_socksUsePassword->isChecked());
        kcfg_socksUsername->setEnabled(on && kcfg_socksUsePassword->isChecked());
    }

    void ProxyPref::usernamePasswordToggled(bool on)
    {
        kcfg_socksPassword->setEnabled(on && kcfg_socksEnabled->isChecked());
        kcfg_socksUsername->setEnabled(on && kcfg_socksEnabled->isChecked());
    }
}

