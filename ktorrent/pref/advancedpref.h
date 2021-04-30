/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KT_ADVNACEDPREF_HH
#define KT_ADVNACEDPREF_HH

#include "ui_advancedpref.h"
#include <interfaces/prefpageinterface.h>

namespace kt
{
class AdvancedPref : public PrefPageInterface, public Ui_AdvancedPref
{
    Q_OBJECT
public:
    AdvancedPref(QWidget *parent);
    ~AdvancedPref() override;

    void loadSettings() override;
    void loadDefaults() override;

public Q_SLOTS:
    void onDiskPreallocToggled(bool on);
};
}

#endif

// kate: indent-mode cstyle; indent-width 4; replace-tabs on; mixed-indent off;
