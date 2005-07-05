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
#include "bencoder.h"
#include <libutil/file.h>

namespace bt
{

	BEncoder::BEncoder(File* fptr) : fptr(fptr)
	{}


	BEncoder::~BEncoder()
	{}

	void BEncoder::beginDict()
	{
		fptr->write("d",1);
	}
	
	void BEncoder::beginList()
	{
		fptr->write("l",1);
	}
	
	void BEncoder::write(int val)
	{
		QString s = QString("i%1e").arg(val);
		fptr->write(s.utf8(),s.length());
	}
	
	void BEncoder::write(const QString & str)
	{
		QString s = QString("%1:%2").arg(str.length()).arg(str);
		fptr->write(s.utf8(),s.length());
	}
	
	void BEncoder::write(const QByteArray & data)
	{
		QString s = QString::number(data.size());
		fptr->write(s.utf8(),s.length());
		fptr->write(":",1);
		fptr->write(data.data(),data.size());
	}

	void BEncoder::write(const Uint8* data,Uint32 size)
	{
		QString s = QString::number(size) + ":";
		fptr->write(s.utf8(),s.length());
		fptr->write(data,size);
	}
	
	void BEncoder::end()
	{
		fptr->write("e",1);
	}
}
