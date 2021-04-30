/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTFILTEREDITOR_H
#define KTFILTEREDITOR_H

#include <QDialog>
#include <QSortFilterProxyModel>

#include "ui_filtereditor.h"

namespace kt
{
class Filter;
class FilterList;
class CoreInterface;
class FeedList;
class FeedWidgetModel;

class TestFilterModel : public QSortFilterProxyModel
{
public:
    TestFilterModel(Filter *filter, FeedWidgetModel *source, QObject *parent);
    ~TestFilterModel();

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

private:
    Filter *filter;
    FeedWidgetModel *feed_model;
};

/**
    Dialog to edit filters
*/
class FilterEditor : public QDialog, public Ui_FilterEditor
{
public:
    FilterEditor(Filter *filter, FilterList *filters, FeedList *feeds, CoreInterface *core, QWidget *parent);
    ~FilterEditor();

    void onOK();
    void checkOKButton();
    void test();

private:
    bool okIsPossible();
    void applyOnFilter(Filter *f);

private:
    Filter *filter;
    Filter *test_filter;
    CoreInterface *core;
    FeedList *feeds;
    FeedWidgetModel *test_model;
    TestFilterModel *filter_model;
    FilterList *filters;

    QPushButton *okButton;
};

}

#endif
