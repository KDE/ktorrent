/*
    SPDX-FileCopyrightText: 2010 Jonas Lundqvist <jonas@gannon.se>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <torrent/globals.h>

#include "magnetgeneratorpluginsettings.h"
#include "magnetgeneratorprefwidget.h"

using namespace bt;

namespace kt
{
MagnetGeneratorPrefWidget::MagnetGeneratorPrefWidget(QWidget *parent)
    : PrefPageInterface(MagnetGeneratorPluginSettings::self(), i18n("Magnet Generator"), QStringLiteral("kt-magnet"), parent)
{
    setupUi(this);
    connect(kcfg_customtracker, &QCheckBox::toggled, this, &MagnetGeneratorPrefWidget::customTrackerToggled);
    connect(kcfg_torrenttracker, &QCheckBox::toggled, this, &MagnetGeneratorPrefWidget::torrentTrackerToggled);
    kcfg_tr->setEnabled(MagnetGeneratorPluginSettings::customtracker());
}

MagnetGeneratorPrefWidget::~MagnetGeneratorPrefWidget()
{
}

void MagnetGeneratorPrefWidget::customTrackerToggled(bool on)
{
    if (on)
        kcfg_torrenttracker->setCheckState(Qt::Unchecked);

    kcfg_tr->setEnabled(on);
}

void MagnetGeneratorPrefWidget::torrentTrackerToggled(bool on)
{
    if (on) {
        kcfg_customtracker->setCheckState(Qt::Unchecked);
        kcfg_tr->setEnabled(!on);
    }
}

}
