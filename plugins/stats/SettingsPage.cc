/***************************************************************************
 *   Copyright © 2007 by Krzysztof Kundzicz                                *
 *   athantor@gmail.com                                                    *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/

#include <SettingsPage.h>

namespace kt
{

    SettingsPage::SettingsPage(QWidget* p) :
        PrefPageInterface(StatsPluginSettings::self(), i18nc("@title:window", "Statistics"), QStringLiteral("view-statistics"), p)
    {
        setupUi(this);
        connect(kcfg_UpdateEveryGuiUpdates, SIGNAL(valueChanged(int)), this, SLOT(UpdGuiUpdatesToMs(int)));
        UpdGuiUpdatesToMs(0);
    }

    SettingsPage::~SettingsPage()
    {
        disconnect(kcfg_UpdateEveryGuiUpdates);
    }

    void SettingsPage::updateSettings()
    {
        emit Applied();
    }

    void SettingsPage::UpdGuiUpdatesToMs(int)
    {
        UpdMsLbl->setText(i18n("(= %1 ms)", (kcfg_UpdateEveryGuiUpdates->value()) * Settings::guiUpdateInterval())) ;
    }

} //ns end
