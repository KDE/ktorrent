/*
    SPDX-FileCopyrightText: 2010 Jonas Lundqvist <jonas@gannon.se>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MAGNETGENERATORPREFWIDGET_H
#define MAGNETGENERATORPREFWIDGET_H

#include "ui_magnetgeneratorprefwidget.h"
#include <interfaces/prefpageinterface.h>

namespace kt
{
class MagnetGeneratorPrefWidget : public PrefPageInterface, public Ui_MagnetGeneratorPrefWidget
{
    Q_OBJECT
public:
    MagnetGeneratorPrefWidget(QWidget *parent = nullptr);
    ~MagnetGeneratorPrefWidget() override;

private Q_SLOTS:
    void customTrackerToggled(bool on);
    void torrentTrackerToggled(bool on);
};

}

#endif
