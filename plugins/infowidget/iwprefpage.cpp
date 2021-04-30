/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "iwprefpage.h"
#include "infowidgetpluginsettings.h"

namespace kt
{
IWPrefPage::IWPrefPage(QWidget *parent)
    : PrefPageInterface(InfoWidgetPluginSettings::self(), i18n("Info Widget"), QStringLiteral("ktip"), parent)
{
    setupUi(this);
}

IWPrefPage::~IWPrefPage()
{
}
}
