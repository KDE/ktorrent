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
        ManageFiltersDlg(Feed* feed, FilterList* filters, SyndicationActivity* act, QWidget* parent);
        ~ManageFiltersDlg();

        void add();
        void remove();
        void removeAll();
        void newFilter();
        void activeSelectionChanged(const QItemSelection& sel, const QItemSelection& desel);
        void availableSelectionChanged(const QItemSelection& sel, const QItemSelection& desel);

    private:
        void accept() override;

    private:
        Feed* feed;
        FilterList* filters;
        FilterListModel* active;
        FilterListModel* available;
        SyndicationActivity* act;
    };

}

#endif
