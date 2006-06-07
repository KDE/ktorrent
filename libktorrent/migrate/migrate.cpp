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
#include <kurl.h>
#include <klocale.h>
#include <util/log.h>
#include <util/error.h>
#include <util/fileops.h>
#include <util/functions.h>
#include <torrent/globals.h>
#include "migrate.h"
#include "ccmigrate.h"
#include "cachemigrate.h"

namespace bt
{

	Migrate::Migrate()
	{}


	Migrate::~Migrate()
	{}

	void Migrate::migrate(const Torrent & tor,const QString & tor_dir,const QString & sdir)
	{
		// check if directory exists
		if (!bt::Exists(tor_dir))
			throw Error(i18n("The directory %1 does not exist").arg(tor_dir));
		
		// make sure it ends with a /
		QString tdir = tor_dir;
		if (!tdir.endsWith(bt::DirSeparator()))
			tdir += bt::DirSeparator();
		
		// see if the current_chunks file exists
		if (bt::Exists(tdir + "current_chunks"))
		{
			// first see if it isn't a download started by a post-mmap version
			if (!IsPreMMap(tdir + "current_chunks"))
			{
				// it's not pre, so it must be post, so just return
				Out() << "No migrate needed" << endl;
				return;
			}
			
			MigrateCurrentChunks(tor,tdir + "current_chunks"); 
		}
		
		// now we need to migrate t
		if (IsCacheMigrateNeeded(tor,tdir + "cache" + bt::DirSeparator()))
		{
			MigrateCache(tor,tdir + "cache" + bt::DirSeparator(),sdir);
		}
	}
	
	
	
}
