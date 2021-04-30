/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTFILTERLIST_H
#define KTFILTERLIST_H

#include "filterlistmodel.h"

namespace kt
{
/**
    Model to keep track of all filters
*/
class FilterList : public FilterListModel
{
public:
    FilterList(QObject *parent);
    ~FilterList();

    void saveFilters(const QString &file);
    void loadFilters(const QString &file);
    void filterEdited(Filter *filter);
};

}

#endif
