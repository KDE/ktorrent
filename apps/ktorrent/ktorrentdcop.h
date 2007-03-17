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
	virtual QValueList<int> getTorrentNumbers(int type = 3);
	virtual QCStringList getTorrentInfo(int tornumber);
	virtual int getFileCount(int tornumber);
	virtual QCStringList getInfo();
	virtual QCStringList getFileNames(int tornumber);
	virtual QValueList<int> getFilePriorities(int tornumber);
	virtual void setFilePriority(int tornumber, int index, int priority);
	virtual void start(int tornumber);
	virtual void stop(int tornumber, bool user);
	virtual void remove(int tornumber, bool del_data);
	virtual void announce(int tornumber);
	virtual QCString dataDir();
	virtual int maxDownloads();
	virtual int maxSeeds();
	virtual int maxConnections();
	virtual int maxUploadRate();
	virtual int maxDownloadRate();
	virtual bool keepSeeding();
	virtual bool showSystemTrayIcon();
	virtual QValueList<int> intSettings();
	virtual bool isBlockedIP(QString ip);
	virtual void openTorrentSilentlyDir(const QString & file, const QString & savedir);

};

#endif
