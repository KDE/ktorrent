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
#include "torrent.h"
#include "chunk.h"
#include "cache.h"
#include <util/functions.h>

namespace bt
{

	Cache::Cache(Torrent & tor,const QString & data_dir) : tor(tor),data_dir(data_dir)
	{
		if (!data_dir.endsWith(bt::DirSeparator()))
			this->data_dir += bt::DirSeparator();
	}


	Cache::~Cache()
	{}


	void Cache::changeDataDir(const QString & ndir)
	{
		data_dir = ndir;
	}
}
