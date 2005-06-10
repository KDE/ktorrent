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
#ifndef BTGLOBALS_H
#define BTGLOBALS_H



class QString;
class QHostAddress;

namespace bt
{
	class Log;

	typedef unsigned long long Uint64;
	typedef unsigned long Uint32;
	typedef unsigned short Uint16;
	typedef unsigned char Uint8;

	typedef long long Int64;
	typedef long Int32;
	typedef short Int16;
	typedef char Int8;
	
	const Uint32 MAX_MSGLEN = 9 + 131072;
	const Uint16 MIN_PORT = 6881;
	const Uint16 MAX_PORT = 6889;
	const Uint32 MAX_PIECE_LEN = 16384;
	
	const Uint8 CHOKE = 0;
	const Uint8 UNCHOKE = 1;
	const Uint8 INTERESTED = 2;
	const Uint8 NOT_INTERESTED = 3;
	const Uint8 HAVE = 4;
	const Uint8 BITFIELD = 5;
	const Uint8 REQUEST = 6;
	const Uint8 PIECE = 7;
	const Uint8 CANCEL = 8;

	void WriteUint64(Uint8* buf,Uint32 off,Uint64 val);
	Uint64 ReadUint64(const Uint8* buf,Uint64 off);
	
	void WriteUint32(Uint8* buf,Uint32 off,Uint32 val);
	Uint32 ReadUint32(const Uint8* buf,Uint32 off);
	
	void WriteUint16(Uint8* buf,Uint32 off,Uint16 val);
	Uint16 ReadUint16(const Uint8* buf,Uint32 off);

	/*
	void WriteInt64(Uint8* buf,Uint32 off,Int64 val);
	Int64 ReadInt64(const Uint8* buf,Uint32 off);
	
	void WriteInt32(Uint8* buf,Uint32 off,Int32 val);
	Int32 ReadInt32(const Uint8* buf,Uint32 off);
	
	void WriteInt16(Uint8* buf,Uint32 off,Int16 val);
	Int16 ReadInt16(const Uint8* buf,Uint32 off);*/
	
	Uint32 GetCurrentTime();

	QHostAddress LookUpHost(const QString & host);
	QString DirSeparator();

	class Globals
	{
	public:
		virtual ~Globals();
		
		void initLog(const QString & file);
		void setDebugMode(bool on) {debug_mode = on;}
		bool isDebugModeSet() const {return debug_mode;}

		Log & getLog() {return *log;}
		static Globals & instance() {return inst;}
	private:
		Globals();
		
		bool debug_mode;
		Log* log;

		static Globals inst;
		
		friend Log& Out();
	};

};

#endif
