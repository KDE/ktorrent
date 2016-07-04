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
#include "weekdaymodel.h"

#include <QLocale>

namespace kt
{

    WeekDayModel::WeekDayModel(QObject* parent)
        : QAbstractListModel(parent)
    {
        for (int i = 0; i < 7; i++)
            checked[i] = false;
    }


    WeekDayModel::~WeekDayModel()
    {
    }


    int WeekDayModel::rowCount(const QModelIndex& parent) const
    {
        if (!parent.isValid())
            return 7;
        else
            return 0;
    }

    QVariant WeekDayModel::data(const QModelIndex& index, int role) const
    {
        if (index.row() < 0 || index.row() >= 7)
            return QVariant();

        if (role == Qt::DisplayRole)
        {
            return QLocale::system().dayName(index.row() + 1);
        }
        else if (role == Qt::CheckStateRole)
        {
            return checked[index.row()] ? Qt::Checked : Qt::Unchecked;
        }

        return QVariant();
    }

    bool WeekDayModel::setData(const QModelIndex& index, const QVariant& value, int role)
    {
        if (!index.isValid() || index.row() < 0 || index.row() >= 7)
            return false;

        if (role == Qt::CheckStateRole)
        {
            checked[index.row()] = (Qt::CheckState)value.toUInt() == Qt::Checked;
            dataChanged(index, index);
            return true;
        }
        return false;
    }

    Qt::ItemFlags WeekDayModel::flags(const QModelIndex& index) const
    {
        if (!index.isValid() || index.row() >= 7 || index.row() < 0)
            return QAbstractItemModel::flags(index);
        else
            return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
    }

    QList<int> WeekDayModel::checkedDays() const
    {
        QList<int> ret;
        for (int i = 0; i < 7; i++)
            if (checked[i])
                ret << (i + 1);
        return ret;
    }

}
