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

#include <QStringList>
#include <KLocalizedString>

#include <arpa/inet.h>
#include <net/address.h>
#include <util/log.h>
#include <util/error.h>
#include "ipfilterlist.h"

using namespace bt;

namespace kt
{


    IPFilterList::IPFilterList()
        : bt::BlockListInterface()
    {
    }


    IPFilterList::~IPFilterList()
    {
    }

    bool IPFilterList::blocked(const net::Address& addr) const
    {
        quint32 ip = 0;
        if (addr.isIPv4Mapped())
            ip = addr.convertIPv4Mapped().toIPv4Address();
        else if (addr.ipVersion() != 4)
            return false;
        else
            ip = addr.toIPv4Address();

        foreach (const Entry& e, ip_list)
        {
            if (e.start <= ip && ip <= e.end)
                return true;
        }

        return false;
    }

    bool IPFilterList::parseIPWithWildcards(const QString& str, bt::Uint32& start, bt::Uint32& end)
    {
        QStringList ip_comps = str.split(QLatin1Char('.'));
        if (ip_comps.count() != 4)
            return false;

        bt::Uint32 ip = 0;
        bt::Uint32 mask = 0;
        for (int i = 0; i < 4; i++)
        {
            if (ip_comps[i] == QStringLiteral("*"))
            {
                mask |= 0xFF000000 >> (8 * i);
            }
            else
            {
                bool ok = false;
                bt::Uint32 n = ip_comps[i].toUInt(&ok);
                if (!ok || n > 255)
                    return false;

                ip |= (n & 0x000000FF) << (8 * (3 - i));
            }
        }

        if (mask == 0xFFFFFFFF || mask == 0x00FFFFFF || mask == 0x0000FFFF || mask == 0x000000FF || mask == 0)
        {
            start = ip;
            end = ip | mask;
            return true;
        }
        else
        {
            return false;
        }
    }

    bool IPFilterList::addIP(const QString& str)
    {
        Entry e;
        e.string_rep = str;
        if (!parseIPWithWildcards(str, e.start, e.end))
            return false;

        ip_list.append(e);
        return true;
    }


    bool IPFilterList::addIPRange(const QString& str)
    {
        QStringList range = str.split(QLatin1Char('-'));
        if (range.count() != 2)
            return false;

        net::Address start;
        net::Address end;
        if (!start.setAddress(range[0]) || !end.setAddress(range[1]))
            return false;

        Entry e = {str, start.toIPv4Address(), end.toIPv4Address()};
        ip_list.append(e);
        insertRow(ip_list.count());
        return true;
    }


    bool IPFilterList::add(const QString& str)
    {
        int pos = ip_list.count();
        beginInsertRows(QModelIndex(), pos, pos + 1);
        bool ret = addIPRange(str) || addIP(str);
        endInsertRows();
        return ret;
    }

    void IPFilterList::remove(int row, int count)
    {
        if (row < 0 || row + count > rowCount())
            return;

        removeRows(row, count, QModelIndex());
    }

    void IPFilterList::clear()
    {
        ip_list.clear();
        endResetModel();
    }

    int IPFilterList::rowCount(const QModelIndex& parent) const
    {
        if (!parent.isValid())
            return ip_list.count();
        else
            return 0;
    }

    QVariant IPFilterList::data(const QModelIndex& index, int role) const
    {
        if (!index.isValid() || index.row() >= ip_list.count() || index.row() < 0)
            return QVariant();

        const Entry& e = ip_list.at(index.row());
        switch (role)
        {
        case Qt::DisplayRole:
        case Qt::EditRole:
            return e.string_rep;
        default:
            return QVariant();
        }
    }

    bool IPFilterList::setData(const QModelIndex& index, const QVariant& value, int role)
    {
        if (!index.isValid() || index.row() >= ip_list.count() || index.row() < 0 || role != Qt::EditRole)
            return false;

        Entry& e = ip_list[index.row()];
        QString str = value.toString();
        QStringList range = str.split(QLatin1Char('-'));
        if (range.count() != 2)
        {
            if (!parseIPWithWildcards(str, e.start, e.end))
                return false;

            e.string_rep = str;
        }
        else
        {
            net::Address start;
            net::Address end;
            if (!start.setAddress(range[0]) || !end.setAddress(range[1]))
                return false;

            e.start = start.toIPv4Address();
            e.end = end.toIPv4Address();
            e.string_rep = str;
        }

        emit dataChanged(index, index);
        return true;
    }

    bool IPFilterList::insertRows(int row, int count, const QModelIndex& parent)
    {
        if (parent.isValid())
            return false;

        beginInsertRows(QModelIndex(), row, row + count - 1);
        endInsertRows();
        return true;
    }

    bool IPFilterList::removeRows(int row, int count, const QModelIndex& parent)
    {
        if (parent.isValid())
            return false;

        beginRemoveRows(QModelIndex(), row, row + count - 1);
        for (int i = 0; i < count; i++)
            ip_list.removeAt(row);
        endRemoveRows();
        return true;
    }

    Qt::ItemFlags IPFilterList::flags(const QModelIndex& index) const
    {
        if (!index.isValid() || index.row() >= ip_list.count() || index.row() < 0)
            return QAbstractItemModel::flags(index);
        else
            return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    }
}
