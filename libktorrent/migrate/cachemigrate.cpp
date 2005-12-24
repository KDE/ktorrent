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
#include <qfileinfo.h>
#include <util/fileops.h>
#include <util/functions.h>
#include <torrent/torrent.h>
#include "cachemigrate.h"


namespace bt
{

	bool IsCacheMigrateNeeded(const Torrent & tor,const QString & cache)
	{
		// mutli files allways need to be migrated
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
		bt::Move(cache,output_dir + tor.getNameSuggestion());
		bt::SymLink(output_dir + tor.getNameSuggestion(),cache);
	}
	
	static void MigrateMultiCache(const Torrent & tor,const QString & cache,const QString & output_dir)
	{
		// make the output dir if it does not exists
		if (!bt::Exists(output_dir + tor.getNameSuggestion()))
			bt::MakeDir(output_dir + tor.getNameSuggestion());
		
		QString odir = output_dir + tor.getNameSuggestion() + bt::DirSeparator();
		QString cdir = cache;
	}

	void MigrateCache(const Torrent & tor,const QString & cache,const QString & output_dir)
	{
		if (!tor.isMultiFile())
			MigrateSingleCache(tor,cache,output_dir);
		else
			MigrateMultiCache(tor,cache,output_dir);
	}
}
