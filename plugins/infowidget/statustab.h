/*
    SPDX-FileCopyrightText: 2005-2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef STATUSTAB_H
#define STATUSTAB_H

#include "ui_statustab.h"
#include <QPointer>
#include <QWidget>
#include <interfaces/torrentinterface.h>

namespace kt
{
class StatusTab : public QWidget, public Ui_StatusTab
{
    Q_OBJECT

public:
    StatusTab(QWidget *parent);
    ~StatusTab() override;

public Q_SLOTS:
    void changeTC(bt::TorrentInterface *tc);
    void update();
    void maxRatioChanged(double v);
    void useRatioLimitToggled(bool on);
    void useTimeLimitToggled(bool on);
    void maxTimeChanged(double v);
    void linkActivated(const QString &link);

private:
    void maxRatioUpdate();
    void maxSeedTimeUpdate();

private:
    QPointer<bt::TorrentInterface> curr_tc;
};
}

#endif
