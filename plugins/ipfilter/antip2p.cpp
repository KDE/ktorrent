/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
#include "antip2p.h"

#include <torrent/globals.h>
#include <util/log.h>
#include <util/constants.h>
#include <util/mmapfile.h>

#include <kglobal.h>
#include <kstandarddirs.h>

#include <qstring.h>

using namespace bt;

namespace kt
{
	AntiP2P::AntiP2P()
	{
		file = new MMapFile();
		if(! file->open(KGlobal::dirs()->saveLocation("data","ktorrent") + "level1.dat", MMapFile::READ) )
		{
			Out() << "Anti-p2p file not loaded." << endl;
			file = 0;
			return;
		}
  	}

  	AntiP2P::~AntiP2P()
  	{
		if(file)
			delete file;
	}
	
	void AntiP2P::loadHeader()
	{
		if(!file)
			return;
	}
	
	bool AntiP2P::exists()
	{
		return file == 0;
	}
}
