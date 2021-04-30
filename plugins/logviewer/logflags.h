/*
    SPDX-FileCopyrightText: 2006 Ivan Vasić <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTLOGFLAGS_H
#define KTLOGFLAGS_H

#include <QAbstractTableModel>
#include <QList>
#include <util/constants.h>

class QString;

namespace kt
{
/**
 * Class to read/save logging messages flags.
 * @author Ivan Vasic <ivasic@gmail.com>
 */
class LogFlags : public QAbstractTableModel
{
    Q_OBJECT

public:
    LogFlags();
    ~LogFlags() override;

    static LogFlags &instance();

    /// Checks current flags with arg. Return true if message should be shown
    bool checkFlags(unsigned int arg);

    /// Updates flags from Settings::
    void updateFlags();

    /// Makes line rich text according to arg level.
    QString getFormattedMessage(unsigned int arg, const QString &line);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool removeRows(int row, int count, const QModelIndex &parent) override;
    bool insertRows(int row, int count, const QModelIndex &parent) override;

private Q_SLOTS:
    void registered(const QString &sys);
    void unregistered(const QString &sys);

private:
    QString flagToString(bt::Uint32 flag) const;

private:
    struct LogFlag {
        QString name;
        bt::Uint32 id;
        bt::Uint32 flag;
    };
    QList<LogFlag> log_flags;
};

}

#endif
