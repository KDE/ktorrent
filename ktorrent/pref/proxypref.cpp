/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "proxypref.h"
#include "settings.h"

namespace kt
{
ProxyPref::ProxyPref(QWidget *parent)
    : PrefPageInterface(Settings::self(), i18n("Proxy"), QStringLiteral("preferences-system-network-proxy"), parent)
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
