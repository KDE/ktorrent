/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
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
#ifndef KTVIEWITEM_HH
#define KTVIEWITEM_HH

#include <QTreeWidgetItem>

namespace kt
{
	class View;
	class TorrentInterface;

	/**
	 * An item in the View
	 * */
	class ViewItem : public QTreeWidgetItem
	{
	public:
		ViewItem(kt::TorrentInterface* tc,View* parent);
		virtual ~ViewItem();

		/// Update all fields
		void update(bool init = false);

		/// Comparison operator for sorting
		bool operator < (const QTreeWidgetItem & other) const;

		kt::TorrentInterface* tc;
	private:
		// cached values to avoid unneeded updates
		TorrentStatus status;
		Uint64 bytes_downloaded; 
		Uint64 bytes_uploaded;
		Uint64 total_bytes_to_download;
		Uint32 download_rate;
		Uint32 upload_rate;
		Uint32 seeders_total;
		Uint32 seeders_connected_to;
		Uint32 leechers_total;
		Uint32 leechers_connected_to;
		double percentage;
		float share_ratio;
		Uint32 runtime_dl;
		Uint32 runtime_ul;
		int eta;
	};
}


#endif

