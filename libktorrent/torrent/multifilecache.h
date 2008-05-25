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
#ifndef BTMULTIFILECACHE_H
#define BTMULTIFILECACHE_H


#include <util/ptrmap.h>
#include "cache.h"
#include "settings.h"

namespace bt
{
	class DNDFile;
	class CacheFile;

	/**
	 * @author Joris Guisson
	 * @brief Cache for multi file torrents
	 *
	 * This class manages a multi file torrent cache. Everything gets stored in the
	 * correct files immediately. 
	 */
	class MultiFileCache : public Cache
	{
		QString cache_dir,output_dir;
		PtrMap<Uint32,CacheFile> files;
		PtrMap<Uint32,DNDFile> dnd_files;
	public:
		MultiFileCache(Torrent& tor,const QString & tmpdir,const QString & datadir,bool custom_output_name);
		virtual ~MultiFileCache();

		virtual void changeTmpDir(const QString& ndir);
		virtual void create();
		virtual void load(Chunk* c);
		virtual void save(Chunk* c);
		virtual bool prep(Chunk* c);
		virtual void close();
		virtual void open();
		virtual QString getOutputPath() const;
		virtual void changeOutputPath(const QString & outputpath);
		virtual KIO::Job* moveDataFiles(const QString & ndir);
		virtual void moveDataFilesCompleted(KIO::Job* job);
		virtual void preallocateDiskSpace(PreallocationThread* prealloc);
		virtual bool hasMissingFiles(QStringList & sl);
		virtual void deleteDataFiles();
		virtual Uint64 diskUsage();
	private:
		void touch(TorrentFile & tf);
		virtual void downloadStatusChanged(TorrentFile*, bool);
		QString guessDataDir();
		void saveFirstAndLastChunk(TorrentFile* tf,const QString & src_file,const QString & dst_file);
		void recreateFile(TorrentFile* tf,const QString & dnd_file,const QString & output_file);
	};

}

#endif
