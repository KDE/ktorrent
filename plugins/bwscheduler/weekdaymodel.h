/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTWEEKDAYMODEL_H
#define KTWEEKDAYMODEL_H

#include <QAbstractListModel>

namespace kt
{
/**
    Model to display the days of a week in a list view. The weekdays are checkable.
    @author
*/
class WeekDayModel : public QAbstractListModel
{
    Q_OBJECT
public:
    WeekDayModel(QObject *parent);
    ~WeekDayModel() override;

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /// Get all the days which have been checked
    QList<int> checkedDays() const;

private:
    bool checked[7];
};

}

#endif
