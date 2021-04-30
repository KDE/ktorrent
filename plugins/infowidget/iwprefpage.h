/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTIWPREFPAGE_HH
#define KTIWPREFPAGE_HH

#include "ui_iwprefpage.h"
#include <interfaces/prefpageinterface.h>

namespace kt
{
class IWPrefPage : public PrefPageInterface, public Ui_IWPrefPage
{
    Q_OBJECT
public:
    IWPrefPage(QWidget *parent);
    ~IWPrefPage() override;
};
}

#endif
