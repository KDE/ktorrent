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
#include "bencoder.h"
#include <libutil/file.h>

namespace bt
{
	

	BEncoderFileOutput::BEncoderFileOutput(File* fptr) : fptr(fptr)
	{
	}

	void BEncoderFileOutput::write(const char* str,Uint32 len)
	{
		if (!fptr)
			return;

		fptr->write(str,len);
	}

	////////////////////////////////////

	BEncoder::BEncoder(File* fptr) : out(0),del(true)
	{
		out = new BEncoderFileOutput(fptr);
	}

	BEncoder::BEncoder(BEncoderOutput* out) : out(out),del(false)
	{
	}


	BEncoder::~BEncoder()
	{
		if (del)
			delete out;
	}

	void BEncoder::beginDict()
	{
		if (!out) return;
		
		out->write("d",1);
	}
	
	void BEncoder::beginList()
	{
		if (!out) return;
		
		out->write("l",1);
	}
	
	void BEncoder::write(Uint32 val)
	{
		if (!out) return;
		
		QString s = QString("i%1e").arg(val);
		out->write(s.utf8(),s.length());
	}
	
	void BEncoder::write(const QString & str)
	{
		if (!out) return;
		
		QString s = QString("%1:%2").arg(str.length()).arg(str);
		out->write(s.utf8(),s.length());
	}
	
	void BEncoder::write(const QByteArray & data)
	{
		if (!out) return;
		
		QString s = QString::number(data.size());
		out->write(s.utf8(),s.length());
		out->write(":",1);
		out->write(data.data(),data.size());
	}

	void BEncoder::write(const Uint8* data,Uint32 size)
	{
		if (!out) return;
		
		QString s = QString::number(size) + ":";
		out->write(s.utf8(),s.length());
		out->write((const char*)data,size);
	}
	
	void BEncoder::end()
	{
		if (!out) return;
		
		out->write("e",1);
	}
}
