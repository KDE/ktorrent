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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef KTORRENTCORE_H
#define KTORRENTCORE_H

#include <qobject.h>

namespace bt
{
	class TorrentControl;
};

/**
@author Joris Guisson
*/
class KTorrentCore : public QObject
{
	Q_OBJECT
public:
	KTorrentCore();
	virtual ~KTorrentCore();
	
	
	void loadTorrents();
	void setMaxDownloads(int max);
	void setKeepSeeding(bool ks);
	void onExit();

public slots:
	void load(const QString & file);
	void remove(bt::TorrentControl* tc);
	
signals:
	void torrentAdded(bt::TorrentControl* tc);
	void torrentRemoved(bt::TorrentControl* tc);
	void finished(bt::TorrentControl* tc);
	
private:
	QString findNewTorrentDir() const;
	int getNumRunning() const;
	
private slots:
	void torrentFinished(bt::TorrentControl* tc);
	
private:
	QPtrList<bt::TorrentControl> downloads;
	QString data_dir;
	int max_downloads;
	bool keep_seeding;
};

#endif
