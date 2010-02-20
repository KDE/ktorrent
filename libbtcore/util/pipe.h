/***************************************************************************
 *   Copyright (C) 2009 by Joris Guisson                                   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#ifndef BT_PIPE_H
#define BT_PIPE_H

#include <btcore_export.h>
#include <util/constants.h>

namespace bt
{

	/**
		Cross platform pipe implementation, uses socketpair on unix and a TCP connection over the localhost in windows.
	*/
	class BTCORE_EXPORT Pipe
	{
	public:
		Pipe();
		virtual ~Pipe();
		
		/// Get the reader socket
		int readerSocket() const {return reader;}
		
		/// Get the writer socket
		int writerSocket() const {return writer;}
		
		/// Write data to the write end of the pipe
		int write(const Uint8* data,int len);
		
		/// Read data from the read end of the pipe
		int read(Uint8* buffer,int max_len);
		
	protected:
		int reader;
		int writer;
	};

}

#endif // BT_PIPE_H
