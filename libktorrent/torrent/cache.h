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
#ifndef BTCACHE_H
#define BTCACHE_H

namespace bt
{
	class Torrent;
	class Chunk;

	/**
	 * @author Joris Guisson
	 * @brief Manages the temporary data
	 *
	 * Interface for a class which manages downloaded data.
	 * Subclasses should implement the load and save methods.
	 */
	class Cache
	{
	protected:
		Torrent & tor;
		QString tmpdir;
		QString datadir;
	public:
		Cache(Torrent & tor,const QString & tmpdir,const QString & datadir);
		virtual ~Cache();

		/**
		 * Changes the tmp dir. All data files should allready been moved.
		 * This just modifies the tmpdir variable.
		 * @param ndir The new tmpdir
		 */
		virtual void changeTmpDir(const QString & ndir);
		
		/**
		 * Load a chunk into memory. If something goes wrong,
		 * an Error should be thrown.
		 * @param c The Chunk
		 */
		virtual void load(Chunk* c) = 0;

		/**
		 * Save a chunk to disk. If something goes wrong,
		 * an Error should be thrown.
		 * @param c The Chunk
		 */
		virtual void save(Chunk* c) = 0;

		/**
		 * Create all the data files to store the data.
		 */
		virtual void create() = 0;
	};

}

#endif
