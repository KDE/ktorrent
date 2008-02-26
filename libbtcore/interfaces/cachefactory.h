/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
#ifndef BTCACHEFACTORY_H
#define BTCACHEFACTORY_H

#include <QString>
#include <btcore_export.h>

namespace bt
{
	class Cache;
	class Torrent;

	/**
	 * Factory to create Cache objects. If you want a custom Cache you need to derive from this class
	 * and implement the create method to create your own custom Caches.
	 * @author Joris Guisson
	*/
	class BTCORE_EXPORT CacheFactory
	{
	public:
		CacheFactory();
		virtual ~CacheFactory();

		/**
		 * Create a custom Cache
		 * @param tor The Torrent
		 * @param tmpdir The temporary directory (should be used to store information about the torrent)
		 * @param datadir The data directory, where to store the data
		 * @return 
		 */
		virtual Cache* create(Torrent & tor,const QString & tmpdir,const QString & datadir) = 0;
	};

}

#endif
