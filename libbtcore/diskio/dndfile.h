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
#ifndef BTDNDFILE_H
#define BTDNDFILE_H

#include <qstring.h>
#include <util/constants.h>

namespace bt
{
	class TorrentFile;

	/**
	 * @author Joris Guisson <joris.guisson@gmail.com>
	 * 
	 * Special file where we keep the first and last chunk of a file which is marked as do not download.
	 * THe first and last chunk of a file will most certainly be partial chunks.
	 */
	class DNDFile
	{
	public:
		DNDFile(const QString & path,const TorrentFile* tf,Uint32 chunk_size);
		virtual ~DNDFile();
		
		/// Change the path of the file
		void changePath(const QString & npath);
		
		/**
		 * CHeck integrity of the file, create it if it doesn't exist.
		 */
		void checkIntegrity();
		
		/**
		 * Read the (partial)first chunk into a buffer.
		 * @param buf The buffer
		 * @param off Offset off chunk
		 * @param size How many bytes to read of the first chunk
		*/
		Uint32 readFirstChunk(Uint8* buf,Uint32 off,Uint32 size);
		
		/**
		 * Read the (partial)last chunk into a buffer.
		 * @param buf The buffer
		 * @param off Offset off chunk
		 * @param size How many bytes to read of the last chunk
		 */
		Uint32 readLastChunk(Uint8* buf,Uint32 off,Uint32 size);
		
		/**
		 * Write the partial first chunk.
		 * @param buf The buffer
		 * @param off Offset into partial chunk
 		 * @param size Size to write
		 */
		void writeFirstChunk(const Uint8* buf,Uint32 off,Uint32 size);
		
		/**
		 * Write the partial last chunk.
		 * @param buf The buffer
		 * @param off Offset into partial chunk
		 * @param size Size to write
		 */
		void writeLastChunk(const Uint8* buf,Uint32 off,Uint32 size);
		
	private:
		void create();

	private:
		QString path;
		Uint32 first_size;
		Uint32 last_size;
	};

}

#endif
