/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "qmpref.h"
#include "settings.h"

namespace kt
{
QMPref::QMPref(QWidget *parent)
    : PrefPageInterface(Settings::self(), i18n("Queue Manager"), QStringLiteral("preferences-log"), parent)
{
    setupUi(this);
    connect(kcfg_manuallyControlTorrents, &QCheckBox::toggled, this, &QMPref::onControlTorrentsManuallyToggled);
    kcfg_stallTimer->setSuffix(i18n(" min"));
}

QMPref::~QMPref()
{
}

void QMPref::loadSettings()
{
    kcfg_stallTimer->setEnabled(Settings::decreasePriorityOfStalledTorrents() && !Settings::manuallyControlTorrents());
    kcfg_maxDownloads->setDisabled(Settings::manuallyControlTorrents());
    kcfg_maxSeeds->setDisabled(Settings::manuallyControlTorrents());
    kcfg_decreasePriorityOfStalledTorrents->setDisabled(Settings::manuallyControlTorrents());
}

void QMPref::loadDefaults()
{
    loadSettings();
}

void QMPref::onControlTorrentsManuallyToggled(bool on)
{
    kcfg_stallTimer->setEnabled(kcfg_decreasePriorityOfStalledTorrents->isChecked() && !on);
}

}
