/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KT_BTPREF_H
#define KT_BTPREF_H

#include "ui_btpref.h"
#include <interfaces/prefpageinterface.h>

namespace kt
{
class BTPref : public PrefPageInterface, public Ui_BTPref
{
    Q_OBJECT
public:
    BTPref(QWidget *parent);
    ~BTPref() override;

    void loadSettings() override;
};
}

#endif // KT_BTPREF_H
