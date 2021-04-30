/*
    SPDX-FileCopyrightText: 2020 Alexander Trufanov <trufanovan@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "scanforlostfilesprefpage.h"
#include "scanforlostfilesplugin.h"

#include <KLocalizedString>

#include "scanforlostfilespluginsettings.h"
#include <interfaces/coreinterface.h>
#include <util/functions.h>

namespace kt
{
ScanForLostFilesPrefPage::ScanForLostFilesPrefPage(ScanForLostFilesPlugin *plugin, QWidget *parent)
    : PrefPageInterface(ScanForLostFilesPluginSettings::self(), i18nc("plugin name", "Scan for lost files"), QStringLiteral("edit-find"), parent)
    , m_plugin(plugin)
{
    setupUi(this);
}

ScanForLostFilesPrefPage::~ScanForLostFilesPrefPage()
{
}

void ScanForLostFilesPrefPage::loadSettings()
{
    kcfg_ScanForLostFilesWidgetPosition->setCurrentIndex(ScanForLostFilesPluginSettings::scanForLostFilesWidgetPosition());
}

void ScanForLostFilesPrefPage::loadDefaults()
{
    kcfg_ScanForLostFilesWidgetPosition->setCurrentIndex(0);
}

void ScanForLostFilesPrefPage::saveSettings()
{
    ScanForLostFilesPluginSettings::setScanForLostFilesWidgetPosition(kcfg_ScanForLostFilesWidgetPosition->currentIndex());
    ScanForLostFilesPluginSettings::self()->save();
}

void ScanForLostFilesPrefPage::updateSettings()
{
    saveSettings();
    m_plugin->updateScanForLostFiles();
}

bool ScanForLostFilesPrefPage::customWidgetsChanged()
{
    return true;
}

}
