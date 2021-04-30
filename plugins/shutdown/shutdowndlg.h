/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KT_SHUTDOWNDLG_H
#define KT_SHUTDOWNDLG_H

#include "shutdownruleset.h"
#include "ui_shutdowndlg.h"
#include <QDialog>

namespace kt
{
class CoreInterface;
class ShutdownTorrentModel;

class ShutdownDlg : public QDialog, public Ui_ShutdownDlg
{
    Q_OBJECT
public:
    ShutdownDlg(ShutdownRuleSet *rules, CoreInterface *core, QWidget *parent);
    ~ShutdownDlg() override;

    void accept() override;
    void reject() override;

private Q_SLOTS:
    void timeToExecuteChanged(int idx);

private:
    Action indexToAction(int idx);
    int actionToIndex(Action act);

private:
    ShutdownRuleSet *rules;
    ShutdownTorrentModel *model;
};

}

#endif // KT_SHUTDOWNDLG_H
