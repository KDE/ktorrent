/***************************************************************************
 *   Copyright (C) 2009 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
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

#include "qmpref.h"
#include "settings.h"

namespace kt
{
    QMPref::QMPref(QWidget* parent) : PrefPageInterface(Settings::self(), i18n("Queue Manager"), QStringLiteral("kt-queue-manager"), parent)
    {
        setupUi(this);
        connect(kcfg_manuallyControlTorrents, &QCheckBox::toggled, this, &QMPref::onControlTorrentsManuallyToggled);
        kcfg_stallTimer->setSuffix(i18n(" min"));
    }

    QMPref::~QMPref()
    {}

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
