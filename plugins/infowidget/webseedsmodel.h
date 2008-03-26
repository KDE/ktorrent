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
#ifndef KTWEBSEEDSMODEL_H
#define KTWEBSEEDSMODEL_H

#include <QAbstractTableModel>
#include <util/constants.h>

namespace bt
{
	class TorrentInterface;
}

namespace kt
{

	/**
		@author
	*/
	class WebSeedsModel : public QAbstractTableModel
	{
		Q_OBJECT
	
	public:
		WebSeedsModel(QObject* parent);
		virtual ~WebSeedsModel();
		
		
		/**
		 * Change the current torrent.
		 * @param tc 
		 */
		void changeTC(bt::TorrentInterface* tc);
		
		/**
		 *  See if we need to update the model
		 */
		bool update();
		
		virtual int rowCount(const QModelIndex & parent) const;
		virtual int columnCount(const QModelIndex & parent) const;
		virtual QVariant headerData(int section, Qt::Orientation orientation,int role) const;
		virtual QVariant data(const QModelIndex & index, int role) const;
		
	private:
		struct Item
		{
			QString status;
			bt::Uint64 downloaded;
			bt::Uint32 speed;
		};
		bt::TorrentInterface* curr_tc;
		QList<Item> items;
	};

}

#endif
