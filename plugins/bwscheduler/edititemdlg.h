/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2007 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTEDITITEMDLG_H
#define KTEDITITEMDLG_H

#include "ui_edititemdlg.h"
#include <QDialog>

namespace kt
{
struct ScheduleItem;
class Schedule;

/**
    @author
*/
class EditItemDlg : public QDialog, public Ui_EditItemDlg
{
    Q_OBJECT
public:
    EditItemDlg(Schedule *schedule, ScheduleItem *item, bool new_item, QWidget *parent);
    ~EditItemDlg() override;

    /**
     * accept will only work if the item does not conflict
     **/
    void accept() override;

private Q_SLOTS:
    void fromChanged(const QTime &time);
    void toChanged(const QTime &time);
    void startDayChanged(int idx);
    void endDayChanged(int idx);
    void suspendedChanged(bool on);
    void screensaverLimitsToggled(bool on);

private:
    void fillItem();

private:
    Schedule *schedule;
    ScheduleItem *item;
};

}

#endif
