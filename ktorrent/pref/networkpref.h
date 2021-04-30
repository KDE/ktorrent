/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTNETWORKPREF_H
#define KTNETWORKPREF_H

#include "ui_networkpref.h"
#include <interfaces/prefpageinterface.h>

namespace kt
{
/**
    Preference page for network settings.
*/
class NetworkPref : public PrefPageInterface, public Ui_NetworkPref
{
    Q_OBJECT
public:
    NetworkPref(QWidget *parent);
    ~NetworkPref() override;

    void loadSettings() override;
    void loadDefaults() override;
    void updateSettings() override;
Q_SIGNALS:
    void calculateRecommendedSettings();

private Q_SLOTS:
    void utpEnabled(bool on);
    void onlyUseUtpEnabled(bool on);
};

}

#endif
