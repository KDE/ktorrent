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
#ifndef KTTORRENTMIGRATOR_H
#define KTTORRENTMIGRATOR_H

#include <QObject>
#include <QMap>
#include <QStringList>
#include <ktcore_export.h>
#include <util/constants.h>
#include "ui_torrentmigratordlg.h"


namespace kt
{
	class QueueManager;

	/**
		Class to find old torrents and migrate them to the KDE4 version.
	*/
	class TorrentMigratorDlg : public QDialog,public Ui_TorrentMigratorDlg
	{
		Q_OBJECT
	public:
		TorrentMigratorDlg(QWidget* parent);
		virtual ~TorrentMigratorDlg();
		
		/**
		 * Find all the torrents which can be migrated.
		 * @param qman The QueueManager
		 * @return The number found
		 */
		bt::Uint32 findTorrentsToBeMigrated();
		
		/**
		 * Migrate the torrents found.This will show the dialog.
		 * @param qman The QueueManager
		 */
		void migrateFoundTorrents(QueueManager* qman);
		
		/// Get all successfully imported torrents (their new torX dir)
		const QStringList & getSuccessFullImports() const {return success;}
	private:
		void doTorrent(const QString & tor,bt::Uint32 idx,QueueManager* qman);
		
	private:
		QMap<QString,bt::Uint32> todo;
		QStringList success;
	};

}

#endif
