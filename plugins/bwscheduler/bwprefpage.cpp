/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "bwprefpage.h"
#include "bwschedulerpluginsettings.h"

namespace kt
{
BWPrefPage::BWPrefPage(QWidget *parent)
    : PrefPageInterface(SchedulerPluginSettings::self(), i18n("Scheduler"), QStringLiteral("kt-bandwidth-scheduler"), parent)
{
    setupUi(this);
}

BWPrefPage::~BWPrefPage()
{
}

void BWPrefPage::loadDefaults()
{
    kcfg_screensaverDownloadLimit->setEnabled(SchedulerPluginSettings::screensaverLimits());
    kcfg_screensaverUploadLimit->setEnabled(SchedulerPluginSettings::screensaverLimits());
}

void BWPrefPage::loadSettings()
{
    kcfg_screensaverDownloadLimit->setEnabled(SchedulerPluginSettings::screensaverLimits());
    kcfg_screensaverUploadLimit->setEnabled(SchedulerPluginSettings::screensaverLimits());
}

void BWPrefPage::updateSettings()
{
    colorsChanged();
}

}
