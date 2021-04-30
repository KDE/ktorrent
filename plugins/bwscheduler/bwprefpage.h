/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTBWPREFPAGE_H
#define KTBWPREFPAGE_H

#include "ui_bwprefpage.h"
#include <interfaces/prefpageinterface.h>

namespace kt
{
/**
    @author
*/
class BWPrefPage : public PrefPageInterface, public Ui_BWPrefPage
{
    Q_OBJECT
public:
    BWPrefPage(QWidget *parent);
    ~BWPrefPage() override;

    void loadDefaults() override;
    void loadSettings() override;
    void updateSettings() override;

Q_SIGNALS:
    void colorsChanged();
};

}

#endif
