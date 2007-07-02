/***************************************************************************
 *   Copyright (C) 2005 by                                                 *
 *   Joris Guisson <joris.guisson@gmail.com>                               *
 *   Ivan Vasic <ivasic@gmail.com>                                         *
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
#include <math.h>
#include <util/log.h>
#include <util/bitset.h>
#include <util/functions.h>
#include "globals.h"
#include "torrentfile.h"

namespace bt
{

	TorrentFile::TorrentFile() : TorrentFileInterface(QString::null,0),missing(false),filetype(UNKNOWN)
	{}

	TorrentFile::TorrentFile(Uint32 index,const QString & path,
							 Uint64 off,Uint64 size,Uint64 chunk_size)
	: TorrentFileInterface(path,size),index(index),cache_offset(off),missing(false),filetype(UNKNOWN)
	{
		first_chunk = off / chunk_size;
		first_chunk_off = off % chunk_size;
		if (size > 0)
			last_chunk = (off + size - 1) / chunk_size;
		else
			last_chunk = first_chunk;
		last_chunk_size = (off + size) - last_chunk * chunk_size;
		priority = old_priority = NORMAL_PRIORITY;
	}
	
	TorrentFile::TorrentFile(const TorrentFile & tf)
		: TorrentFileInterface(QString::null,0)
	{
		index = tf.getIndex();
		path = tf.getPath();
		size = tf.getSize();
		cache_offset = tf.getCacheOffset();
		first_chunk = tf.getFirstChunk();
		first_chunk_off = tf.getFirstChunkOffset();
		last_chunk = tf.getLastChunk();
		last_chunk_size = tf.getLastChunkSize();
		old_priority = priority = tf.getPriority();
		missing = tf.isMissing();
		filetype = UNKNOWN;
	}

	TorrentFile::~TorrentFile()
	{}

	void TorrentFile::setDoNotDownload(bool dnd)
	{
		if (priority != EXCLUDED && dnd)
		{
			if(m_emitDlStatusChanged)
				old_priority = priority;
			
			priority = EXCLUDED;
			
			if(m_emitDlStatusChanged)
				emit downloadPriorityChanged(this,priority,old_priority);	
		}
		if (priority == EXCLUDED && (!dnd))
		{
			if(m_emitDlStatusChanged)
				old_priority = priority;
			
			priority = NORMAL_PRIORITY;
			
			if(m_emitDlStatusChanged)
				emit downloadPriorityChanged(this,priority,old_priority);
		}
	}
	
	void TorrentFile::emitDownloadStatusChanged()
	{
		// only emit when old_priority is not equal to the new priority
		if (priority != old_priority)
			emit downloadPriorityChanged(this,priority,old_priority);
	}


	bool TorrentFile::isMultimedia() const
	{
		if (filetype == UNKNOWN)
		{
			if (IsMultimediaFile(getPath()))
			{
				filetype = MULTIMEDIA;
				return true;
			}
			else
			{
				filetype = NORMAL;
				return false;
			}
		}
		return filetype == MULTIMEDIA;
	}

	void TorrentFile::setPriority(Priority newpriority)
	{
		if(priority != newpriority)
		{
			if (priority == EXCLUDED)
			{
				setDoNotDownload(false);
			}
			if (newpriority == EXCLUDED)
			{
				setDoNotDownload(true);
			}
			else
			{
				old_priority = priority;
				priority = newpriority;
				emit downloadPriorityChanged(this,newpriority,old_priority);
			}
		}
	}

	TorrentFile & TorrentFile::operator = (const TorrentFile & tf)
	{
		index = tf.getIndex();
		path = tf.getPath();
		size = tf.getSize();
		cache_offset = tf.getCacheOffset();
		first_chunk = tf.getFirstChunk();
		first_chunk_off = tf.getFirstChunkOffset();
		last_chunk = tf.getLastChunk();
		last_chunk_size = tf.getLastChunkSize();
		priority = tf.getPriority();
		missing = tf.isMissing();
		return *this;
	}

	TorrentFile TorrentFile::null;

	
	Uint64 TorrentFile::fileOffset(Uint32 cindex,Uint64 chunk_size) const
	{
		Uint64 off = 0;
		if (getFirstChunkOffset() == 0)
		{
			off = (cindex - getFirstChunk()) * chunk_size;
		}
		else
		{
			if (cindex - this->getFirstChunk() > 0)
				off = (cindex - this->getFirstChunk() - 1) * chunk_size;
			if (cindex > 0)
				off += (chunk_size - this->getFirstChunkOffset());
		}
		return off;
	}
	
	void TorrentFile::updateNumDownloadedChunks(const BitSet & bs)
	{
		float p = getDownloadPercentage();
		num_chunks_downloaded = 0;
		bool prev = preview;
		preview = true;
		for (Uint32 i = first_chunk;i <= last_chunk;i++)
		{
			if (bs.get(i))
			{
				num_chunks_downloaded++;
			}
			else if (i == first_chunk || i == first_chunk + 1)
			{
				preview = false;
			}
		}
		preview = isMultimedia() && preview;
		
		float np = getDownloadPercentage();
		if (fabs(np - p) >= 0.01f)
			downloadPercentageChanged(np);
		
		if (prev != preview)
			previewAvailable(preview);
	}
}
#include "torrentfile.moc"
