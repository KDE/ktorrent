/*
    SPDX-FileCopyrightText: 2010 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KT_PROPERTIESDLG_H
#define KT_PROPERTIESDLG_H

#include "ui_propertiesdlg.h"
#include <QDialog>

namespace bt
{
class TorrentInterface;
}

namespace kt
{
/**
    Extender which shows properties about a torrent.
*/
class PropertiesDlg : public QDialog, public Ui_PropertiesDlg
{
    Q_OBJECT
public:
    PropertiesDlg(bt::TorrentInterface *tc, QWidget *parent);
    ~PropertiesDlg() override;

public Q_SLOTS:
    void moveOnCompletionEnabled(bool on);

private:
    void accept() override;

private:
    bt::TorrentInterface *tc;
};

}

#endif // KT_PROPERTIESEXTENDER_H
