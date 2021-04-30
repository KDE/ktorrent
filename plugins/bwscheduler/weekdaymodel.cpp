/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "weekdaymodel.h"

#include <QLocale>

namespace kt
{
WeekDayModel::WeekDayModel(QObject *parent)
    : QAbstractListModel(parent)
{
    for (int i = 0; i < 7; i++)
        checked[i] = false;
}

WeekDayModel::~WeekDayModel()
{
}

int WeekDayModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return 7;
    else
        return 0;
}

QVariant WeekDayModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= 7)
        return QVariant();

    if (role == Qt::DisplayRole) {
        return QLocale::system().dayName(index.row() + 1);
    } else if (role == Qt::CheckStateRole) {
        return checked[index.row()] ? Qt::Checked : Qt::Unchecked;
    }

    return QVariant();
}

bool WeekDayModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() < 0 || index.row() >= 7)
        return false;

    if (role == Qt::CheckStateRole) {
        checked[index.row()] = (Qt::CheckState)value.toUInt() == Qt::Checked;
        dataChanged(index, index);
        return true;
    }
    return false;
}

Qt::ItemFlags WeekDayModel::flags(const QModelIndex &index) const
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
