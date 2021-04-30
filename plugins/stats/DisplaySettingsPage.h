/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTDISPLAYSETTINGSPAGE_H
#define KTDISPLAYSETTINGSPAGE_H

#include "ui_DisplaySettings.h"
#include <interfaces/prefpageinterface.h>

namespace kt
{
/**
    @author
*/

class DisplaySettingsPage : public PrefPageInterface, public Ui_DisplaySettingsWgt
{
public:
    DisplaySettingsPage(QWidget *parent);
    ~DisplaySettingsPage() override;
};

}

#endif
