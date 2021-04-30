/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTRECOMMENDEDSETTINGSDLG_H
#define KTRECOMMENDEDSETTINGSDLG_H

#include "ui_recommendedsettingsdlg.h"
#include <QDialog>
#include <util/constants.h>

namespace kt
{
/**
    Dialog to compute the best settings
*/
class RecommendedSettingsDlg : public QDialog, public Ui_RecommendedSettingsDlg
{
    Q_OBJECT
public:
    RecommendedSettingsDlg(QWidget *parent);
    ~RecommendedSettingsDlg() override;

private Q_SLOTS:
    void calculate();
    void apply();
    void avgSpeedSlotToggled(bool on);
    void simTorrentsToggled(bool on);
    void slotsToggled(bool on);
    void uploadBWChanged(int val);
    void downloadBWChanged(int val);

private:
    void saveState(KSharedConfigPtr cfg);
    void loadState(KSharedConfigPtr cfg);

public:
    bt::Uint32 max_upload_speed;
    bt::Uint32 max_download_speed;
    bt::Uint32 max_conn_tor;
    bt::Uint32 max_conn_glob;
    bt::Uint32 max_downloads;
    bt::Uint32 max_seeds;
    bt::Uint32 max_slots;
};

}

#endif
