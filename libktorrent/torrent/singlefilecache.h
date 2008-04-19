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
#ifndef BTSINGLEFILECACHE_H
#define BTSINGLEFILECACHE_H

#include "cache.h"

namespace bt
{
	class CacheFile;


	/**
	 * @author Joris Guisson
	 * @brief Cache for single file torrents
	 *
	 * This class implements Cache for a single file torrent
	 */
	class SingleFileCache : public Cache
	{
		QString cache_file;
		QString output_file;
		CacheFile* fd;
	public:
		SingleFileCache(Torrent& tor,const QString & tmpdir,const QString & datadir);
		virtual ~SingleFileCache();

		virtual bool prep(Chunk* c);
		virtual void load(Chunk* c);
		virtual void save(Chunk* c);
		virtual void create();
		virtual void close();
		virtual void open();
		virtual void changeTmpDir(const QString & ndir);
		virtual KIO::Job* moveDataFiles(const QString & ndir);
		virtual void moveDataFilesCompleted(KIO::Job* job);
		virtual void changeOutputPath(const QString& outputpath);
		virtual QString getOutputPath() const {return output_file;}
		virtual void preallocateDiskSpace(PreallocationThread* prealloc);
		virtual bool hasMissingFiles(QStringList & sl);
		virtual void deleteDataFiles();
		virtual Uint64 diskUsage();
	};

}

#endif
