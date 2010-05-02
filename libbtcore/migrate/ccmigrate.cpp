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
#include "ccmigrate.h"
#include <klocale.h>
#include <util/log.h>
#include <util/file.h>
#include <util/error.h>
#include <util/array.h>
#include <util/bitset.h>
#include <util/fileops.h>
#include <download/downloader.h>
#include <torrent/torrent.h>
#include <torrent/globals.h>
#include <download/chunkdownload.h>
#include "btversion.h"

namespace bt
{
	bool IsPreMMap(const QString & current_chunks)
	{
		File fptr;
		if (!fptr.open(current_chunks,"rb"))
			return false;

		CurrentChunksHeader chdr;
		fptr.read(&chdr,sizeof(CurrentChunksHeader));
		if (chdr.magic != CURRENT_CHUNK_MAGIC)
		{
			// magic number not good, so pre
			return true;
		}
		
		if (chdr.major >= 2 || (chdr.major == 1 && chdr.minor >= 2))
		{
			// version number is 1.2 or greater
			return false;
		}
		
		return false;
	}
	
	void MigrateCurrentChunks(const Torrent & tor,const QString & current_chunks)
	{
		Out(SYS_GEN|LOG_DEBUG) << "Migrating very old current chunks files is no longer supported" << endl;
		bt::Delete(current_chunks,true);
	}

}
