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
#include "cache.h"
#include <util/functions.h>
#include <util/log.h>
#include <torrent/torrent.h>
#include "chunk.h"
#include "cachefile.h"
#include "piecedata.h"
#include <peer/peermanager.h>

namespace bt
{
	bool Cache::preallocate_files = true;
	bool Cache::preallocate_fully = false;

	Cache::Cache(Torrent & tor,const QString & tmpdir,const QString & datadir)
	: tor(tor),tmpdir(tmpdir),datadir(datadir),mmap_failures(0)
	{
		if (!datadir.endsWith(bt::DirSeparator()))
			this->datadir += bt::DirSeparator();

		if (!tmpdir.endsWith(bt::DirSeparator()))
			this->tmpdir += bt::DirSeparator();
		
		preexisting_files = false;
	}


	Cache::~Cache()
	{
		clearPieceCache();
	}


	void Cache::changeTmpDir(const QString & ndir)
	{
		tmpdir = ndir;
	}
	
	bool Cache::mappedModeAllowed()
	{
#ifndef Q_WS_WIN
		return MaxOpenFiles() - bt::PeerManager::getTotalConnections() > 100;
#else
		return true; //there isn't a file handle limit on windows
#endif
	}
	
	Job* Cache::moveDataFiles(const QMap<TorrentFileInterface*,QString> & files)
	{
		Q_UNUSED(files);
		return 0;
	}
	
	void Cache::moveDataFilesFinished(const QMap<TorrentFileInterface*,QString> & files,Job* job)
	{
		Q_UNUSED(files);
		Q_UNUSED(job);
	}
	
	PieceDataPtr Cache::findPiece(Chunk* c,Uint32 off,Uint32 len)
	{
		QMultiMap<Chunk*,PieceData*>::iterator i = piece_cache.find(c);
		while (i != piece_cache.end() && i.key() == c)
		{
			PieceData* cp = i.value();
			if (cp->offset() == off && cp->length() == len)
				return PieceDataPtr(cp);
			i++;
		}
		
		return 0;
	}
	
	void Cache::insertPiece(Chunk* c,PieceData* p)
	{
		piece_cache.insert(c,p);
	}
	
	void Cache::clearPieces(Chunk* c)
	{
		QMultiMap<Chunk*,PieceData*>::iterator i = piece_cache.find(c);
		while (i != piece_cache.end() && i.key() == c)
		{
			PieceData* cp = i.value();
			delete cp;
			i = piece_cache.erase(i);
		}
	}
	
	void Cache::clearPieceCache()
	{
		QMultiMap<Chunk*,PieceData*>::iterator i = piece_cache.begin();
		while (i != piece_cache.end())
		{
			PieceData* cp = i.value();
			delete cp;
			i++;
		}
		piece_cache.clear();
	}

	void Cache::clearPiece(PieceData* p)
	{
		Chunk* c = p->parentChunk();
		QMultiMap<Chunk*,PieceData*>::iterator i = piece_cache.find(p->parentChunk());
		while (i != piece_cache.end() && i.key() == c)
		{
			if (i.value() == p)
			{
				PieceData* cp = i.value();
				delete cp;
				piece_cache.erase(i);
				break;
			}
			i++;
		}
	}
	
	void Cache::checkMemoryUsage()
	{
		Uint64 mem = 0;
		Uint64 freed = 0;
		QMultiMap<Chunk*,PieceData*>::iterator i = piece_cache.begin();
		while (i != piece_cache.end())
		{
			PieceData* cp = i.value();
			if (!cp->inUse())
			{
				freed += cp->length();
				delete cp;
				i = piece_cache.erase(i);
			}
			else
			{
			//	Out(SYS_GEN|LOG_DEBUG) << "PieceCache: " << i.key()->getIndex() << " " << cp->offset() << " " << cp->length() << " " << cp->ref_count <<  endl;
				mem += cp->length();
				i++;
			}
		}

		if (mem || freed)
			Out(SYS_DIO|LOG_DEBUG) << "Piece cache: memory in use " << BytesToString(mem) << ", memory freed " << BytesToString(freed) << endl;
	}

}
