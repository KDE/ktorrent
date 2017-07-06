/***************************************************************************
*   Copyright (C) 2009 by Joris Guisson                                   *
*   joris.guisson@gmail.com                                               *
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

#ifndef ITEMSELECTIONMODEL_H
#define ITEMSELECTIONMODEL_H

#include <QItemSelectionModel>
#include <QSet>

#include <ktcore_export.h>

namespace kt
{
    /**
     * Selection model which works on internal pointers instead of indexes.
     */
    class KTCORE_EXPORT ItemSelectionModel : public QItemSelectionModel
    {
        Q_OBJECT
    public:
        ItemSelectionModel(QAbstractItemModel* model, QObject* parent);
        ~ItemSelectionModel();

        void select(const QModelIndex& index, QItemSelectionModel::SelectionFlags command) override;
        void select(const QItemSelection& sel, QItemSelectionModel::SelectionFlags command) override;
        void clear() override;
        void reset() override;

    public slots:
        /**
         * Updates the selection after a sort.
         */
        void sorted();

    private:
        void doRange(const QItemSelectionRange r, QItemSelectionModel::SelectionFlags command);

    private:
        QSet<void*> selection;
    };
}

#endif // ITEMSELECTIONMODEL_H
