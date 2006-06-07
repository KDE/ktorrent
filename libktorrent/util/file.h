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
#ifndef BTFILE_H
#define BTFILE_H

#include <stdio.h>
#include <qstring.h>
#include "constants.h"

namespace bt
{

	/**
	 * @author Joris Guisson
	 * @brief Wrapper class for stdio's FILE
	 *
	 * Wrapper class for stdio's FILE.
	 */
	class File
	{
		FILE* fptr;
		QString file;
	public:
		/**
		 * Constructor.
		 */
		File();
		
		/**
		 * Destructor, closes the file.
		 */
		virtual ~File();

		/**
		 * Open the file similar to fopen
		 * @param file Filename
		 * @param mode Mode
		 * @return true upon succes
		 */
		bool open(const QString & file,const QString & mode);
		
		/**
		 * Close the file.
		 */
		void close();
		
		/**
		 * Flush the file.
		 */
		void flush();
		
		/**
		 * Write a bunch of data. If anything goes wrong
		 * an Error will be thrown.
		 * @param buf The data
		 * @param size Size of the data
		 * @return The number of bytes written
		 */
		Uint32 write(const void* buf,Uint32 size);
		
		/**
		 * Read a bunch of data. If anything goes wrong
		 * an Error will be thrown.
		 * @param buf The buffer to store the data
		 * @param size Size of the buffer
		 * @return The number of bytes read
		 */
		Uint32 read(void* buf,Uint32 size);

		enum SeekPos
		{
			BEGIN,
			END,
			CURRENT
		};
		
		/**
		 * Seek in the file.
		 * @param from Position to seek from
		 * @param num Number of bytes to move
		 * @return New position
		 */
		Uint64 seek(SeekPos from,Int64 num);

		/// Check to see if we are at the end of the file.
		bool eof() const;

		/// Get the current position in the file.
		Uint64 tell() const;

		/// Get the error string.
		QString errorString() const;
	};

}

#endif
