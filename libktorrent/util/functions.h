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
#ifndef BTFUNCTIONS_H
#define BTFUNCTIONS_H

#include "constants.h"

class QString;
class QHostAddress;
class KURL;

namespace bt
{

	void WriteUint64(Uint8* buf,Uint32 off,Uint64 val);
	Uint64 ReadUint64(const Uint8* buf,Uint64 off);
	
	void WriteUint32(Uint8* buf,Uint32 off,Uint32 val);
	Uint32 ReadUint32(const Uint8* buf,Uint32 off);
	
	void WriteUint16(Uint8* buf,Uint32 off,Uint16 val);
	Uint16 ReadUint16(const Uint8* buf,Uint32 off);

	
	void WriteInt64(Uint8* buf,Uint32 off,Int64 val);
	Int64 ReadInt64(const Uint8* buf,Uint32 off);
	
	void WriteInt32(Uint8* buf,Uint32 off,Int32 val);
	Int32 ReadInt32(const Uint8* buf,Uint32 off);
	
	void WriteInt16(Uint8* buf,Uint32 off,Int16 val);
	Int16 ReadInt16(const Uint8* buf,Uint32 off);
	
	void UpdateCurrentTime();
	
	extern TimeStamp global_time_stamp;
	
	inline TimeStamp GetCurrentTime() {return global_time_stamp;}
	
	TimeStamp Now();
	
	QHostAddress LookUpHost(const QString & host);
	QString DirSeparator();
	bool IsMultimediaFile(const QString & filename);

	/**
	 * Maximize the file and memory limits using setrlimit.
	 */
	bool MaximizeLimits();
	
	/// Get the maximum number of open files
	Uint32 MaxOpenFiles();
}

#endif
