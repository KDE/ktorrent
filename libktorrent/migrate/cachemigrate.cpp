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
#include <qstringlist.h>
#include <qfileinfo.h>
#include <util/log.h>
#include <util/fileops.h>
#include <util/functions.h>
#include <torrent/torrent.h>
#include <torrent/globals.h>
#include "cachemigrate.h"


namespace bt
{

	bool IsCacheMigrateNeeded(const Torrent & tor,const QString & cache)
	{
		// mutli files always need to be migrated
		if (tor.isMultiFile())
			return true;
		
		// a single file and a symlink do not need to be migrated
		QFileInfo finfo(cache);
		if (finfo.isSymLink())
			return false;
		
		return true;
	}
	
	static void MigrateSingleCache(const Torrent & tor,const QString & cache,const QString & output_dir)
	{
		Out() << "Migrating single cache " << cache << " to " << output_dir << endl;
		
		bt::Move(cache,output_dir + tor.getNameSuggestion());
		bt::SymLink(output_dir + tor.getNameSuggestion(),cache);
	}
	
	static void MakePath(const QString & startdir,const QString & path)
	{
		QStringList sl = QStringList::split(bt::DirSeparator(),path);

		// create all necessary subdirs
		QString ctmp = startdir;
		
		for (Uint32 i = 0;i < sl.count() - 1;i++)
		{
			ctmp += sl[i];
			// we need to make the same directory structure in the cache
			// as the output dir
			if (!bt::Exists(ctmp))
				MakeDir(ctmp);
			
			ctmp += bt::DirSeparator();
		}
	}
	
	static void MigrateMultiCache(const Torrent & tor,const QString & cache,const QString & output_dir)
	{
		Out() << "Migrating multi cache " << cache << " to " << output_dir << endl;
		// if the cache dir is a symlink, everything is OK
		if (QFileInfo(cache).isSymLink())
			return;
		
		QString cache_dir = cache;
		
		
		// make the output dir if it does not exists
		if (!bt::Exists(output_dir + tor.getNameSuggestion()))
			bt::MakeDir(output_dir + tor.getNameSuggestion());
		
		QString odir = output_dir + tor.getNameSuggestion() + bt::DirSeparator();
		QString cdir = cache;
		if (!cdir.endsWith(bt::DirSeparator()))
			cdir += bt::DirSeparator();
		
		// loop over all files in the cache and see if they are symlinks
		for (Uint32 i = 0;i < tor.getNumFiles();i++)
		{
			const TorrentFile & tf = tor.getFile(i);
			QFileInfo fi(cdir + tf.getPath());
			// symlinks are OK
			if (fi.isSymLink())
				continue;
			// make the path if necessary
			MakePath(odir,tf.getPath());
			// no symlink so move to output_dir
			bt::Move(cdir + tf.getPath(),odir + tf.getPath());
			bt::SymLink(odir + tf.getPath(),cdir + tf.getPath());
		}
	}

	void MigrateCache(const Torrent & tor,const QString & cache,const QString & output_dir)
	{
		QString odir = output_dir;
		if (!odir.endsWith(bt::DirSeparator()))
			odir += bt::DirSeparator();
		
		if (!tor.isMultiFile())
			MigrateSingleCache(tor,cache,odir);
		else
			MigrateMultiCache(tor,cache,odir);
	}
}
