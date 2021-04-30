/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTFILTERLISTVIEW_H
#define KTFILTERLISTVIEW_H

#include <QListView>

namespace kt
{
class Filter;
class FilterList;

/**
    List view to display filters.
*/
class FilterListView : public QListView
{
    Q_OBJECT
public:
    FilterListView(FilterList *filters, QWidget *parent);
    ~FilterListView();

    QModelIndexList selectedFilters();

    void itemActivated(const QModelIndex &idx);
    void selectionChanged(const QItemSelection &sel, const QItemSelection &desel) override;

Q_SIGNALS:
    void filterActivated(Filter *filter);
    void enableRemove(bool on);
    void enableEdit(bool on);

private:
    FilterList *filters;
};

}

#endif
