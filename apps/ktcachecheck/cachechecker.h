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
#ifndef DEBUGCACHECHECKER_H
#define DEBUGCACHECHECKER_H

#include <set>
#include <qstring.h>
#include <util/functions.h>

namespace bt
{
	class Torrent;
}

namespace ktdebug
{

	/**
	 * @author Joris Guisson
	*/
	class CacheChecker
	{
	public:
		CacheChecker(bt::Torrent & tor);
		virtual ~CacheChecker();

		void loadIndex(const QString & index_file);
		void fixIndex();
		bool foundFailedChunks() const {return failed_chunks.size() > 0;}
		bool foundExtraChunks() const {return extra_chunks.size() > 0;}
		
		virtual void check(const QString & cache,const QString & index) = 0;
	protected:
		bt::Torrent & tor;
		QString index_file;
		std::set<bt::Uint32>  downloaded_chunks;
		std::set<bt::Uint32> failed_chunks;
		std::set<bt::Uint32> extra_chunks;
	};

	
}

#endif
