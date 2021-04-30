/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTQMPREF_H
#define KTQMPREF_H

#include "ui_qmpref.h"
#include <interfaces/prefpageinterface.h>

namespace kt
{
/**
    Preference page for the queue manager
*/
class QMPref : public PrefPageInterface, public Ui_QMPref
{
    Q_OBJECT
public:
    QMPref(QWidget *parent);
    ~QMPref() override;

    void loadSettings() override;
    void loadDefaults() override;
private Q_SLOTS:
    void onControlTorrentsManuallyToggled(bool on);
};

}

#endif
