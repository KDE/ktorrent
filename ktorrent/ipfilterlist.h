/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTIPFILTERLIST_H
#define KTIPFILTERLIST_H

#include <QAbstractListModel>
#include <QList>

#include <interfaces/blocklistinterface.h>
#include <util/constants.h>

namespace kt
{
/**
    Blocklist for the IPFilterWidget
*/
class IPFilterList : public QAbstractListModel, public bt::BlockListInterface
{
public:
    IPFilterList();
    ~IPFilterList() override;

    bool blocked(const net::Address &addr) const override;

    /// Add an IP address with a mask.
    bool add(const QString &ip);

    /// Remove the IP address at a given row and count items following that
    void remove(int row, int count);

    /// Clear the list
    void clear();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    bool insertRows(int row, int count, const QModelIndex &parent) override;
    bool removeRows(int row, int count, const QModelIndex &parent) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    bool addIP(const QString &str);
    bool addIPRange(const QString &str);
    bool parseIPWithWildcards(const QString &str, bt::Uint32 &start, bt::Uint32 &end);

private:
    struct Entry {
        QString string_rep;
        bt::Uint32 start;
        bt::Uint32 end;
    };

    QList<Entry> ip_list;
};

}

#endif
