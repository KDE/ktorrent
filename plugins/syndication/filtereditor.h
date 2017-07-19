/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

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
        TestFilterModel(Filter* filter, FeedWidgetModel* source, QObject* parent);
        ~TestFilterModel();

        bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
    private:
        Filter* filter;
        FeedWidgetModel* feed_model;
    };

    /**
        Dialog to edit filters
    */
    class FilterEditor : public QDialog, public Ui_FilterEditor
    {
    public:
        FilterEditor(Filter* filter, FilterList* filters, FeedList* feeds, CoreInterface* core, QWidget* parent);
        ~FilterEditor();

        void onOK();
        void checkOKButton();
        void test();

    private:
        bool okIsPossible();
        void applyOnFilter(Filter* f);

    private:
        Filter* filter;
        Filter* test_filter;
        CoreInterface* core;
        FeedList* feeds;
        FeedWidgetModel* test_model;
        TestFilterModel* filter_model;
        FilterList* filters;

        QPushButton *okButton;
    };

}

#endif
