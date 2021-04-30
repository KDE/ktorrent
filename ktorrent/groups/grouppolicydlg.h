/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTGROUPPOLICYDLG_H
#define KTGROUPPOLICYDLG_H

#include "ui_grouppolicydlg.h"
#include <QDialog>

namespace kt
{
class Group;

/**
    @author
*/
class GroupPolicyDlg : public QDialog, public Ui_GroupPolicyDlg
{
public:
    GroupPolicyDlg(Group *group, QWidget *parent);
    ~GroupPolicyDlg() override;

    void accept() override;

private:
    Group *group;
};

}

#endif
