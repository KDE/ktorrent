/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTPROXYPREF_H
#define KTPROXYPREF_H

#include "ui_proxypref.h"
#include <interfaces/prefpageinterface.h>

namespace kt
{
/**
    @author
*/
class ProxyPref : public PrefPageInterface, public Ui_ProxyPref
{
    Q_OBJECT
public:
    ProxyPref(QWidget *parent);
    ~ProxyPref() override;

    void loadDefaults() override;
    void loadSettings() override;
    void updateSettings() override;
private Q_SLOTS:
    void socksEnabledToggled(bool on);
    void usernamePasswordToggled(bool on);
};

}

#endif
