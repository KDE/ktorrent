/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <statspluginsettings.h>
#include "DisplaySettingsPage.h"

namespace kt
{

DisplaySettingsPage::DisplaySettingsPage(QWidget* parent)
    : PrefPageInterface(StatsPluginSettings::self(), i18nc("@title:window", "Display"), QStringLiteral("view-statistics"), parent)
{
    setupUi(this);
}


DisplaySettingsPage::~DisplaySettingsPage()
{
}


}
