/***************************************************************************
 *   Copyright (C) 2005 by                                                 *
 *   Joris Guisson <joris.guisson@gmail.com>                               *
 *   Ivan Vasic <ivasic@gmail.com>                                         *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#ifndef TRAYICON_H
#define TRAYICON_H

#include <ksystemtray.h>

#include "ktorrentcore.h" 
#include "interfaces/torrentinterface.h"
#include <util/constants.h>

using namespace bt; 
class QString;

typedef struct tray_stats
{
	bt::Uint32 download_speed;
	bt::Uint32 upload_speed;
	bt::Uint64 bytes_downloaded;
	bt::Uint64 bytes_uploaded;
	
	
}TrayStats;

/**
 * @author Joris Guisson
 * @author Ivan Vasic
*/
class TrayIcon : public KSystemTray
{
	Q_OBJECT
public:
	TrayIcon(KTorrentCore* tc, QWidget *parent = 0, const char *name = 0);
	~TrayIcon();

	void updateStats(const CurrentStats stats);
	
private slots:
	void finished(kt::TorrentInterface* tc);
	void torrentStoppedByError(kt::TorrentInterface* tc, QString msg);
	void viewChanged(kt::TorrentInterface* tc);

private:
	KTorrentCore* m_core;
};

#endif
