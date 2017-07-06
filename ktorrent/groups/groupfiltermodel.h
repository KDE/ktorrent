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

#ifndef KTGROUPFILTERMODEL_H
#define KTGROUPFILTERMODEL_H

#include <QSortFilterProxyModel>

namespace kt
{
    class Group;
    class ViewModel;

    /**
        Model to filter out torrents based upon group membership
    */
    class GroupFilterModel : public QSortFilterProxyModel
    {
        Q_OBJECT
    public:
        GroupFilterModel(ViewModel* view_model, QObject* parent);
        ~GroupFilterModel();

        bool filterAcceptsColumn(int source_column, const QModelIndex& source_parent) const override;
        bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

        /**
         * Set the group to filter
         * @param g The Group
         * */
        void setGroup(Group* g);

        /**
         * Filter again.
         */
        void refilter();

    private:
        Group* group;
        ViewModel* view_model;
    };

}

#endif
