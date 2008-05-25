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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#ifndef DCOPINTERFACE_H
#define DCOPINTERFACE_H

#include <dcopobject.h>

/**
@author Joris Guisson
*/
class DCOPInterface : virtual public DCOPObject
{
	K_DCOP
k_dcop:
	virtual void startAll(int type = 3) = 0;
	virtual void stopAll(int type = 3) = 0;
	virtual void setMaxDownloads(int max) = 0;
	virtual void setMaxSeeds(int max) = 0;
	virtual void setKeepSeeding(bool ks) = 0;
	virtual void setMaxUploadSpeed(int kbytes_per_sec) = 0;
	virtual void setMaxDownloadSpeed(int kbytes_per_sec) = 0;
	virtual void setMaxConnectionsPerDownload(int max) = 0;
	virtual void setShowSysTrayIcon(bool yes) = 0;
	virtual bool changeDataDir(const QString & new_dir) = 0;
	virtual void openTorrent(const QString & file) = 0;
	virtual void openTorrentSilently(const QString & file) = 0;
	virtual QValueList<int> getTorrentNumbers(int type = 3) = 0;
	virtual QCStringList getTorrentInfo(int tornumber) = 0;
	virtual int getFileCount(int tornumber) = 0;
	virtual QCStringList getInfo() = 0;
	virtual QCStringList getFileNames(int tornumber) = 0;
	virtual QValueList<int> getFilePriorities(int tornumber) = 0;
	virtual void setFilePriority(int tornumber, int index, int priority) = 0;
	virtual void start(int tornumber) = 0;
	virtual void stop(int tornumber, bool user) = 0;
	virtual void remove(int tornumber, bool del_data) = 0;
	virtual void announce(int tornumber) = 0;
	virtual QCString dataDir() = 0;
	virtual int maxDownloads() = 0;
	virtual int maxSeeds() = 0;
	virtual int maxConnections() = 0;
	virtual int maxUploadRate() = 0;
	virtual int maxDownloadRate() = 0;
	virtual bool keepSeeding() = 0;
	virtual bool showSystemTrayIcon() = 0;
	virtual QValueList<int> intSettings() = 0;
	virtual bool isBlockedIP(QString ip) = 0;
	virtual void openTorrentSilentlyDir(const QString & file, const QString & savedir) = 0;
};


#endif
