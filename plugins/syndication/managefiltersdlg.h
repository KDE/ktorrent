/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTMANAGEFILTERSDLG_H
#define KTMANAGEFILTERSDLG_H

#include <QDialog>

#include "ui_managefiltersdlg.h"

namespace kt
{
class Feed;
class FilterList;
class FilterListModel;
class SyndicationActivity;

/**
    Dialog to manage filters for a feed
*/
class ManageFiltersDlg : public QDialog, public Ui_ManageFiltersDlg
{
public:
    ManageFiltersDlg(Feed *feed, FilterList *filters, SyndicationActivity *act, QWidget *parent);
    ~ManageFiltersDlg();

    void add();
    void remove();
    void removeAll();
    void newFilter();
    void activeSelectionChanged(const QItemSelection &sel, const QItemSelection &desel);
    void availableSelectionChanged(const QItemSelection &sel, const QItemSelection &desel);

private:
    void accept() override;

private:
    Feed *feed;
    FilterList *filters;
    FilterListModel *active;
    FilterListModel *available;
    SyndicationActivity *act;
};

}

#endif
