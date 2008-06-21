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
#ifndef KTAPITORRENT_H
#define KTAPITORRENT_H

#include <QObject>

namespace bt
{
	class TorrentInterface;
}

namespace ktapi
{

	/**
		Interface for script to interacts with torrents.
		This will provide a subset of the possibilities of bt::TorrentInterface.
	*/
	class Torrent : public QObject
	{
		Q_OBJECT
	public:
		Torrent(bt::TorrentInterface* ti,QObject* parent);
		virtual ~Torrent();
		
		/// Comparison operator to see if this torrent matches a TorrentInterface
		bool operator == (bt::TorrentInterface* t) const {return ti == t;}
		
	public slots:
		/// Get the name of the torrent
		QString torrentName() const;

	private:
		bt::TorrentInterface* ti;
	};

}

#endif
