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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#ifndef TRAYICON_H
#define TRAYICON_H

#include <ksystemtray.h>
#include "ktorrentcore.h" 
#include "torrent/torrentcontrol.h" 

using namespace bt; 
class QString;

/**
@author Joris Guisson
*/
class TrayIcon : public KSystemTray
{
	Q_OBJECT
public:
	TrayIcon(KTorrentCore* tc, QWidget *parent = 0, const char *name = 0);
	~TrayIcon();

	void updateStats(const QString stats);
	
private slots:
	void finished(bt::TorrentControl* tc);
	void torrentStoppedByError(bt::TorrentControl* tc, QString msg); 

private:
	KTorrentCore* m_core;
};

#endif
