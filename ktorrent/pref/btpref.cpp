/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <KLocalizedString>

#include "btpref.h"
#include "settings.h"

namespace kt
{
BTPref::BTPref(QWidget *parent)
    : PrefPageInterface(Settings::self(), i18n("BitTorrent"), QStringLiteral("application-x-bittorrent"), parent)
{
    setupUi(this);
}

BTPref::~BTPref()
{
}

void BTPref::loadSettings()
{
    kcfg_allowUnencryptedConnections->setEnabled(Settings::useEncryption());
    kcfg_dhtPort->setEnabled(Settings::dhtSupport());
    kcfg_customIP->setEnabled(Settings::useCustomIP());
}

}
