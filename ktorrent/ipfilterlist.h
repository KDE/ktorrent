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
#ifndef KTIPFILTERLIST_H
#define KTIPFILTERLIST_H

#include <QList>
#include <QAbstractListModel>
#include <util/constants.h>
#include <interfaces/blocklistinterface.h>

namespace kt
{
	
	/**
		Blocklist for the IPFilterWidget
	*/
	class IPFilterList : public QAbstractListModel, public bt::BlockListInterface
	{
		struct Entry
		{
			QString string_rep;
			bt::Uint32 ip;   // start_ip if is_range == true
			bt::Uint32 mask; // end_ip if is_range == true;
			bool is_range;
		};
		
	public:
		IPFilterList();
		virtual ~IPFilterList();
		
		virtual bool isBlockedIP(const net::Address & addr);
		virtual bool isBlockedIP(const QString & addr);

		/// Add an IP address with a mask.
		void add(const QString & ip);
		
		/// Remove the IP address at a given row and count items following that
		void remove(int row,int count);
		
		/// Clear the list
		void clear();
		
		virtual int rowCount(const QModelIndex & parent = QModelIndex()) const;
		virtual QVariant data(const QModelIndex & index, int role) const;
		virtual bool setData(const QModelIndex & index,const QVariant & value,int role);
		virtual bool insertRows(int row,int count,const QModelIndex & parent);
		virtual bool removeRows(int row,int count,const QModelIndex & parent);
		virtual Qt::ItemFlags flags(const QModelIndex & index) const;
	private:
		bool decodeIP(const QString & str,bt::Uint32 & ip,bt::Uint32 & mask);
		bool decodeIPRange(const QString &str, bt::Uint32 &start, bt::Uint32 &end);
		bool str2ip(const QString &addr, bt::Uint32 &ip);
	private:
		QList<Entry> ip_list;
	};

}

#endif
