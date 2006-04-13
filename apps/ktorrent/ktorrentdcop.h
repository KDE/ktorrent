/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
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
#ifndef KTORRENTDCOP_H
#define KTORRENTDCOP_H

#include "dcopinterface.h"

class KTorrent;

/**
@author Joris Guisson
*/
class KTorrentDCOP : public QObject,virtual public DCOPInterface
{
	Q_OBJECT
	KTorrent* app;
public:
	KTorrentDCOP(KTorrent* app);
	~KTorrentDCOP();

	virtual bool changeDataDir(const QString& new_dir);
	virtual void openTorrent(const QString& file);
	virtual void openTorrentSilently(const QString & file);
	virtual void setKeepSeeding(bool ks);
	virtual void setMaxConnectionsPerDownload(int max);
	virtual void setMaxDownloads(int max);
	virtual void setMaxSeeds(int max);
	virtual void setMaxUploadSpeed(int kbytes_per_sec);
	virtual void setMaxDownloadSpeed(int kbytes_per_sec);
	virtual void setShowSysTrayIcon(bool yes);
	virtual void startAll(int type = 3);
	virtual void stopAll(int type = 3);

};

#endif
