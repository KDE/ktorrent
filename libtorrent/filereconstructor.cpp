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
#include <stdio.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdatastream.h>
#include <kio/netaccess.h>
#include <qapplication.h>
#include <klocale.h>
#include "chunkmanager.h"
#include "error.h"
#include "filereconstructor.h"
#include "torrent.h"
#include "file.h"

extern QApplication* qApp;

namespace bt
{

	FileReconstructor::FileReconstructor(Torrent & tor,ChunkManager & cman)
	: tor(tor),cman(cman)
	{
	}


	FileReconstructor::~FileReconstructor()
	{}


	void FileReconstructor::reconstruct(const QString & output)
	{
		if (tor.isMultiFile())
			multiReconstruct(output);
		else
			singleReconstruct(output);
	}
	
	void FileReconstructor::multiReconstruct(const QString & dir)
	{
		QString rdir = dir;
		if (!rdir.endsWith(DirSeparator()))
			rdir.append(DirSeparator());
		
		rdir += tor.getNameSuggestion();
		if (!KIO::NetAccess::exists(rdir,true,0) && !KIO::NetAccess::mkdir(rdir,0,0755))
			throw Error(i18n("Can't create directory %1 : %2")
					.arg(rdir).arg(KIO::NetAccess::lastErrorString()));

		if (!rdir.endsWith(DirSeparator()))
			rdir.append(DirSeparator());

		Uint32 cur_off = 0;
		for (Uint32 i = 0;i < tor.getNumFiles();i++)
		{
			Torrent::File file;
			tor.getFile(i,file);
			reconstructFile(rdir + file.path,file.size,cur_off);
		}
	}
	
	void FileReconstructor::createFileDir(const QString & file)
	{
		QFileInfo finfo(file);
		if (KIO::NetAccess::exists(finfo.dirPath(true),true,0))
			return;
		
		QString dir = finfo.dirPath(true);
		if (!KIO::NetAccess::mkdir(dir,0,0755))
			throw Error("Can't make directory " + dir + " : " + KIO::NetAccess::lastErrorString());
	}
	
	void FileReconstructor::reconstructFile(
			const QString & path,Uint32 file_size,Uint32 & cur_off)
	{
		createFileDir(path);

		File output,cache;
		if (!output.open(path,"wb"))
			throw Error(i18n("Can't open file %1 : %2")
					.arg(path).arg(output.errorString()));

		
		if (!cache.open(cman.getCacheFile(),"rb"))
			throw Error(i18n("Can't open file %1 : %2")
					.arg(cman.getCacheFile()).arg(output.errorString()));



		cache.seek(File::BEGIN,cur_off);
		char buf[1024];
		// written is the number of bytes allready written
		Uint32 written = 0;
		while (written < file_size)
		{
			//Out() << "Writing chunk " << cur_chunk << endl;
			
			Uint32 left = file_size - written;
			Uint32 to_write = left < 1024 ? left : 1024;
			cache.read(buf,to_write);
			output.write(buf,to_write);
			cur_off += to_write;
			written += to_write;
			
			completed(cur_off / tor.getChunkSize());
			qApp->processEvents();
		}
		
		cache.close();
		output.close();
	}
	
	void FileReconstructor::singleReconstruct(const QString & file)
	{
		// move the cache file to the output file
		if (!KIO::NetAccess::file_copy(cman.getCacheFile(),file,0644))
			throw Error(QString("Error saving to %1 : %2")
					.arg(file).arg(KIO::NetAccess::lastErrorString()));

	/*	// make a symlink instead of the original file
		// so we can continue seeding the file
		KIO::NetAccess::*/
	}
}

#include "filereconstructor.moc"
