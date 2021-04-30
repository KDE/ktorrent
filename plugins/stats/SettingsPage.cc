/*
    SPDX-FileCopyrightText: 2007 Krzysztof Kundzicz <athantor@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <SettingsPage.h>

namespace kt
{

SettingsPage::SettingsPage(QWidget* p) :
    PrefPageInterface(StatsPluginSettings::self(), i18nc("@title:window", "Statistics"), QStringLiteral("view-statistics"), p)
{
    setupUi(this);
    connect(kcfg_UpdateEveryGuiUpdates, qOverload<int>(&QSpinBox::valueChanged), this, &SettingsPage::UpdGuiUpdatesToMs);
    UpdGuiUpdatesToMs(0);
}

SettingsPage::~SettingsPage()
{
    disconnect(kcfg_UpdateEveryGuiUpdates);
}

void SettingsPage::updateSettings()
{
    Q_EMIT Applied();
}

void SettingsPage::UpdGuiUpdatesToMs(int)
{
    UpdMsLbl->setText(i18n("(= %1 ms)", (kcfg_UpdateEveryGuiUpdates->value()) * Settings::guiUpdateInterval())) ;
}

} //ns end
