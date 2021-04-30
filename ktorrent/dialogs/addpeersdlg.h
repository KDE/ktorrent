/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTADDPEERSDLG_H
#define KTADDPEERSDLG_H

#include "ui_addpeersdlg.h"
#include <QDialog>
#include <interfaces/peersource.h>

namespace bt
{
class TorrentInterface;
}

namespace kt
{
class ManualPeerSource;

/**
    Dialog to manually add peers to a torrent
*/
class AddPeersDlg : public QDialog, public Ui_AddPeersDlg
{
    Q_OBJECT
public:
    AddPeersDlg(bt::TorrentInterface *tc, QWidget *parent);
    ~AddPeersDlg() override;

private Q_SLOTS:
    void addPressed();

private:
    bt::TorrentInterface *tc;
    ManualPeerSource *mps;
};

}

#endif
