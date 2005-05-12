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
#include <sys/time.h> 
#include "globals.h"
#include "log.h"


namespace bt
{
	
	

	Globals Globals::inst;

	void WriteUint32(Uint8* buf,Uint32 off,Uint32 val)
	{
		buf[off + 0] = (Uint8) ((val & 0xFF000000) >> 24);
		buf[off + 1] = (Uint8) ((val & 0x00FF0000) >> 16);
		buf[off + 2] = (Uint8) ((val & 0x0000FF00) >> 8);
		buf[off + 3] = (Uint8) (val & 0x000000FF);
	}
	
	Uint32 ReadUint32(const Uint8* buf,Uint32 off)
	{
		return (buf[off] << 24) | (buf[off+1] << 16) | (buf[off+2] << 8) | buf[off + 3];
	}
	
	void WriteUint16(Uint8* buf,Uint32 off,Uint16 val)
	{
		buf[off + 0] = (Uint8) ((val & 0xFF00) >> 8);
		buf[off + 1] = (Uint8) (val & 0x000FF);
	}

	Uint16 ReadUint16(const Uint8* buf,Uint32 off)
	{
		return (buf[off] << 8) | buf[off + 1];
	}
	
	Uint32 GetCurrentTime()
	{
		struct timeval tv;
		gettimeofday(&tv,0);
		return (Uint32)(tv.tv_sec * 1000 + tv.tv_usec * 0.001);
	}

	Globals::Globals()
	{
		debug_mode = false;
		log = new Log();
	}

	Globals::~ Globals()
	{
		delete log;
	}

	void Globals::initLog(const QString & file)
	{
		log->setOutputFile(file);
		log->setOutputToConsole(debug_mode);
	}

	Log & Out()
	{
		Log & lg = Globals::instance().getLog();
		lg.setOutputToConsole(Globals::instance().isDebugModeSet());
		return lg;
	}
}
;
