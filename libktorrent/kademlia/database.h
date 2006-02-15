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
#ifndef DHTDATABASE_H
#define DHTDATABASE_H

#include <qmap.h>
#include <util/constants.h>
#include <util/array.h>
#include "key.h"


namespace dht
{

	/**
	 * @author Joris Guisson
	 * 
	 * Class where all the key value paires get stored.
	*/
	class Database
	{
		QMap<dht::Key,QByteArray> dmap;
	public:
		Database();
		virtual ~Database();

		/**
		 * Store an entry in the database
		 * @param key The key
		 * @param data The data
		 */
		void store(const dht::Key & key,const QByteArray & data);
		
		
		/**
		 * Find a key value pair, returns a null array if the keys is not present in the map.
		 * @param key The key
		 * @return The data
		 */
		const QByteArray & find(const dht::Key & key) const;
	};

}

#endif
