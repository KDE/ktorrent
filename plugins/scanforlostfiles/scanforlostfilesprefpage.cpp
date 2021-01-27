/***************************************************************************
 *   Copyright (C) 2020 by Alexander Trufanov                              *
 *   trufanovan@gmail.com                                                  *
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
