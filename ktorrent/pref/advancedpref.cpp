/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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

#include <KLocalizedString>

#include "advancedpref.h"
#include "settings.h"

namespace kt
{
    AdvancedPref::AdvancedPref(QWidget* parent) : PrefPageInterface(Settings::self(), i18n("Advanced"), QStringLiteral("configure"), parent)
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
