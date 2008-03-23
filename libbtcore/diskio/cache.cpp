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
#include "cache.h"
#include <util/functions.h>
#include <torrent/torrent.h>
#include "chunk.h"
#include <peer/peermanager.h>

namespace bt
{
	bool Cache::preallocate_files = true;
	bool Cache::preallocate_fully = false;
	bool Cache::preallocate_fs_specific = true;

	Cache::Cache(Torrent & tor,const QString & tmpdir,const QString & datadir)
	: tor(tor),tmpdir(tmpdir),datadir(datadir),mmap_failures(0)
	{
		if (!datadir.endsWith(bt::DirSeparator()))
			this->datadir += bt::DirSeparator();

		if (!tmpdir.endsWith(bt::DirSeparator()))
			this->tmpdir += bt::DirSeparator();
		
		preexisting_files = false;
	}


	Cache::~Cache()
	{}


	void Cache::changeTmpDir(const QString & ndir)
	{
		tmpdir = ndir;
	}
	
	bool Cache::mappedModeAllowed()
	{
		return MaxOpenFiles() - bt::PeerManager::getTotalConnections() < 100;
	}
	
	KJob* Cache::moveDataFiles(const QMap<TorrentFileInterface*,QString> & files)
	{
		Q_UNUSED(files);
		return 0;
	}
	
	void Cache::moveDataFilesFinished(const QMap<TorrentFileInterface*,QString> & files,KJob* job)
	{
		Q_UNUSED(files);
		Q_UNUSED(job);
	}
}
