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
#include <klocale.h>
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
	
	bool IPFilterList::isBlockedIP(const net::Address & addr)
	{
		if (addr.ipVersion() != 4)
			return false;
		
		bt::Uint32 ip = addr.ipAddress().IPv4Addr();
		foreach (const Entry & e,ip_list)
		{
			if ((e.ip | e.mask) == (ip | e.mask))
				return true;
		}
		
		return false;
	}
	
	bool IPFilterList::isBlockedIP(const QString & addr)
	{
		return isBlockedIP(net::Address(addr,7777));
	}
	
	bool IPFilterList::decodeIP(const QString & str,bt::Uint32 & ip,bt::Uint32 & mask)
	{
		QStringList ip_comps = str.split(".");
		if (ip_comps.count() != 4)
			return false;
		
		ip = 0;
		mask = 0;
		for (int i = 0;i < 4;i++)
		{
			if (ip_comps[i] == "*")
				mask |= 0xFF000000 >> (8*i);
			else
			{
				bool ok = false;
				bt::Uint32 n = ip_comps[i].toUInt(&ok);
				if (!ok || n > 255)
					return false;
				
				ip |= (n & 0x000000FF) << (8*(3 - i));
			}
		}
		
		return true;
	}

	void IPFilterList::add(const QString & str)
	{
		bt::Uint32 ip;
		bt::Uint32 mask;
		if (!decodeIP(str,ip,mask))
			throw Error(i18n("Invalid IP address %1 !",str));
		
		foreach (const Entry & e,ip_list)
		{
			if (ip == e.ip && mask == e.mask)
				throw Error(i18n("Duplicate IP address %1 !",str));
		}
		
		Entry e = {str,ip,mask};
		ip_list.append(e);
		insertRow(ip_list.count());
	}
	
	void IPFilterList::remove(int row,int count)
	{
		if (row < 0 || row + count > rowCount())
			return;
		
		removeRows(row,count,QModelIndex());
	}
	
	void IPFilterList::clear()
	{
		ip_list.clear();
		reset();
	}
	
	int IPFilterList::rowCount(const QModelIndex &parent) const
	{
		if (!parent.isValid())
			return ip_list.count();
		else
			return 0;
	}
	
	QVariant IPFilterList::data(const QModelIndex &index, int role) const
	{
		if (!index.isValid() || index.row() >= ip_list.count() || index.row() < 0)
			return QVariant();
		
		const Entry & e = ip_list.at(index.row());
		switch (role)
		{
			case Qt::DisplayRole:
			case Qt::EditRole:
				return e.string_rep;
			default:
				return QVariant();
		}
	}
	
	bool IPFilterList::setData(const QModelIndex & index,const QVariant & value,int role)
	{
		if (!index.isValid() || index.row() >= ip_list.count() || index.row() < 0 || role != Qt::EditRole)
			return false;
		
		QString nip = value.toString();
		bt::Uint32 ip;
		bt::Uint32 mask;
		if (!decodeIP(nip,ip,mask))
			return false;
		
		int idx = 0;
		foreach (const Entry & e,ip_list)
		{
			if (idx != index.row() && ip == e.ip && mask == e.mask)
				return false;
			idx++;
		}
		
		Entry & e = ip_list[index.row()];
		e.ip = ip;
		e.mask = mask;
		e.string_rep = nip;
		emit dataChanged(index,index);
		return true;
	}
	
	bool IPFilterList::insertRows(int row,int count,const QModelIndex & parent)
	{
		if (parent.isValid())
			return false;
		
		beginInsertRows(QModelIndex(),row,row + count - 1);
		endInsertRows();
		return true;
	}
	
	bool IPFilterList::removeRows(int row,int count,const QModelIndex & parent)
	{
		if (parent.isValid())
			return false;
		
		beginRemoveRows(QModelIndex(),row,row + count - 1);
		for (int i = 0;i < count;i++)
			ip_list.removeAt(row);
		endRemoveRows();
		return true;
	}
	
	Qt::ItemFlags IPFilterList::flags(const QModelIndex & index) const
	{
		if (!index.isValid() || index.row() >= ip_list.count() || index.row() < 0)
			return QAbstractItemModel::flags(index);
		else
			return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
	}
}
