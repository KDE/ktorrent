/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SPEEDLIMITSDLG_H
#define SPEEDLIMITSDLG_H

#include "ui_speedlimitsdlg.h"
#include <QDialog>

namespace bt
{
class TorrentInterface;
}

namespace kt
{
class Core;
class SpeedLimitsModel;

/// Dialog to modify the speed limits of a torrent
class SpeedLimitsDlg : public QDialog, public Ui_SpeedLimitsDlg
{
    Q_OBJECT

public:
    SpeedLimitsDlg(bt::TorrentInterface *current, Core *core, QWidget *parent);
    ~SpeedLimitsDlg() override;

protected Q_SLOTS:
    void accept() override;
    void reject() override;
    void apply();
    void spinBoxValueChanged(int);
    void saveState();
    void loadState();

private:
    Core *core;
    SpeedLimitsModel *model;
    bt::TorrentInterface *current;
};
}

#endif
