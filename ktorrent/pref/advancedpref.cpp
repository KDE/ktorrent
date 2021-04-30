/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <KLocalizedString>

#include "advancedpref.h"
#include "settings.h"

namespace kt
{
AdvancedPref::AdvancedPref(QWidget *parent)
    : PrefPageInterface(Settings::self(), i18n("Advanced"), QStringLiteral("preferences-other"), parent)
{
    setupUi(this);
    connect(kcfg_diskPrealloc, &QGroupBox::toggled, this, &AdvancedPref::onDiskPreallocToggled);
    connect(kcfg_requeueMagnets, &QCheckBox::toggled, kcfg_requeueMagnetsTime, &QSpinBox::setEnabled);
}

AdvancedPref::~AdvancedPref()
{
}

void AdvancedPref::loadSettings()
{
    kcfg_fullDiskPrealloc->setEnabled(Settings::diskPrealloc());
    kcfg_numMagnetDownloadingSlots->setValue(Settings::numMagnetDownloadingSlots());
    kcfg_requeueMagnets->setChecked(Settings::requeueMagnets());
    kcfg_requeueMagnetsTime->setEnabled(Settings::requeueMagnets());
    kcfg_requeueMagnetsTime->setValue(Settings::requeueMagnetsTime());
    kcfg_trackerListUrl->setText(Settings::trackerListUrl());
}

void AdvancedPref::loadDefaults()
{
    loadSettings();
}

void AdvancedPref::onDiskPreallocToggled(bool on)
{
    kcfg_fullDiskPrealloc->setEnabled(on);
}
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs on; mixed-indent off;
