/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTGENERALPREF_H
#define KTGENERALPREF_H

#include "ui_generalpref.h"
#include <interfaces/prefpageinterface.h>

namespace kt
{
class GeneralPref : public PrefPageInterface, public Ui_GeneralPref
{
    Q_OBJECT
public:
    GeneralPref(QWidget *parent);
    ~GeneralPref() override;

    void loadSettings() override;
    void loadDefaults() override;
};
}

#endif
