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
#include "bencoder.h"
#include <util/file.h>

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
	
	BEncoderBufferOutput::BEncoderBufferOutput(QByteArray & data) : data(data),ptr(0)
	{
	}

	void BEncoderBufferOutput::write(const char* str,Uint32 len)
	{
		if (ptr + len > data.size())
			data.resize(ptr + len);
		
		for (Uint32 i = 0;i < len;i++)
			data[ptr++] = str[i];
	}

	////////////////////////////////////

	BEncoder::BEncoder(File* fptr) : out(0),del(true)
	{
		out = new BEncoderFileOutput(fptr);
	}

	BEncoder::BEncoder(BEncoderOutput* out) : out(out),del(true)
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
		
		QCString s = QString("i%1e").arg(val).utf8();
		out->write(s,s.length());
	}

	void BEncoder::write(Uint64 val)
	{
		if (!out) return;
		
		QCString s = QString("i%1e").arg(val).utf8();
		out->write(s,s.length());
	}
	
	void BEncoder::write(const QString & str)
	{
		if (!out) return;
		
		QCString u = str.utf8();
		QCString s = QString("%1:").arg(u.length()).utf8();
		out->write(s,s.length());
		out->write(u,u.length());
	}
	
	void BEncoder::write(const QByteArray & data)
	{
		if (!out) return;
		
		QCString s = QString::number(data.size()).utf8();
		out->write(s,s.length());
		out->write(":",1);
		out->write(data.data(),data.size());
	}

	void BEncoder::write(const Uint8* data,Uint32 size)
	{
		if (!out) return;
		
		QCString s = QString("%1:").arg(size).utf8();
		out->write(s,s.length());
		out->write((const char*)data,size);
	}
	
	void BEncoder::end()
	{
		if (!out) return;
		
		out->write("e",1);
	}
}
