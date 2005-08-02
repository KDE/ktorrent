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
#include "torrentfile.h"

namespace bt
{

	TorrentFile::TorrentFile()
	{}

	TorrentFile::TorrentFile(const QString & path,Uint32 off,Uint32 size,Uint32 chunk_size)
	: path(path),size(size),cache_offset(off)
	{
		first_chunk = off / chunk_size;
		first_chunk_off = off % chunk_size;
		last_chunk = (off + size - 1) / chunk_size;
		last_chunk_size = (off + size) - last_chunk * chunk_size;
		do_not_download = false;
	}
	
	TorrentFile::TorrentFile(const TorrentFile & tf) : QObject(0,0)
	{
		path = tf.getPath();
		size = tf.getSize();
		cache_offset = tf.getCacheOffset();
		first_chunk = tf.getFirstChunk();
		first_chunk_off = tf.getFirstChunkOffset();
		last_chunk = tf.getLastChunk();
		last_chunk_size = tf.getLastChunkSize();
		do_not_download = tf.doNotDownload();
	}

	TorrentFile::~TorrentFile()
	{}

	void TorrentFile::setDoNotDownload(bool dnd)
	{
		if (do_not_download != dnd)
		{
			do_not_download = dnd;
			emit downloadStatusChanged(this,!dnd);
		}
	}

	TorrentFile & TorrentFile::operator = (const TorrentFile & tf)
	{
		path = tf.getPath();
		size = tf.getSize();
		cache_offset = tf.getCacheOffset();
		first_chunk = tf.getFirstChunk();
		first_chunk_off = tf.getFirstChunkOffset();
		last_chunk = tf.getLastChunk();
		last_chunk_size = tf.getLastChunkSize();
		do_not_download = tf.doNotDownload();
		return *this;
	}

	TorrentFile TorrentFile::null;
}

#include "torrentfile.moc"
