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
#ifndef KTAPIKTORRENT_H
#define KTAPIKTORRENT_H

#include <QObject>

namespace kt
{
	class CoreInterface;
}

namespace ktapi
{
	class Torrent;

	/**
		Main interface object for scripts. In python for example this thing should be
		imported. 
	*/
	class KTorrent : public QObject
	{
		Q_OBJECT
				
	public:
		KTorrent(kt::CoreInterface* core,QObject* parent);
		virtual ~KTorrent();

	public Q_SLOTS:
		/**
		 * Write a message to the log.
		 * @param str The string to write
		 */
		void log(const QString & str);
		
		/// Get the number of torrents
		int numTorrents();
		
		/// Get a pointer to a torrent
		QObject* torrent(int i);
		
	private slots:
		void torrentAdded(bt::TorrentInterface* ti);
		void torrentRemoved(bt::TorrentInterface* ti);
		
	private:
		kt::CoreInterface* core;
		QList<ktapi::Torrent*> torrents;
	};

}

#endif
