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
#ifndef BTTORRENTFILE_H
#define BTTORRENTFILE_H

#include <qstring.h>
#include <libutil/constants.h>

namespace bt
{

	/**
	 * @author Joris Guisson
	 *
	 * File in a multi file torrent. Keeps track of the path of the file,
	 * it's size, offset into the cache and between which chunks it lies.
	 */
	class TorrentFile
	{
		QString path;
		Uint32 size;
		Uint32 cache_offset;
		Uint32 first_chunk;
		Uint32 first_chunk_off;
		Uint32 last_chunk;
		Uint32 last_chunk_size;
		
	public:
		/**
		 * Default constructor.
		 */
		TorrentFile();
		
		/**
		 * Constructor.
		 * @param path Path of the file
		 * @param off Offset into the torrent
		 * (i.e. how many bytes were all the previous files in the torrent combined)
		 * @param size Size of the file
		 * @param chunk_size Size of each chunk 
		 */
		TorrentFile(const QString & path,Uint32 off,Uint32 size,Uint32 chunk_size);
		
		/**
		 * Copy constructor.
		 * @param tf The TorrentFile to copy
		 */
		TorrentFile(const TorrentFile & tf);
		virtual ~TorrentFile();

		/// Get the path of the file
		QString getPath() const {return path;}

		/// Get the size of the file
		Uint32 getSize() const {return size;}

		/// Get the offset into the torrent
		Uint32 getCacheOffset() const {return cache_offset;}

		/// Get the index of the first chunk in which this file lies
		Uint32 getFirstChunk() const {return first_chunk;}

		/// Get the offset at which the file starts in the first chunk
		Uint32 getFirstChunkOffset() const {return first_chunk_off;}

		/// Get the last chunk of the file
		Uint32 getLastChunk() const {return last_chunk;}

		/// Get how many bytes the files takes up of the last chunk
		Uint32 getLastChunkSize() const {return last_chunk_size;}

		/**
		 * Assignment operator
		 * @param tf The file to copy
		 * @return *this
		 */
		TorrentFile & operator = (const TorrentFile & tf);
	};

}

#endif
