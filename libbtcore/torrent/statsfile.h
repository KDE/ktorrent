/***************************************************************************
 *   Copyright (C) 2005-2010 by Joris Guisson                              *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#ifndef BTSTATSFILE_H
#define BTSTATSFILE_H


#include <KSharedConfig>
#include <btcore_export.h>
#include <util/constants.h>


namespace bt
{

	/**
	 * @brief This class is used for loading/storing torrent stats in a file.
	 * @author Ivan Vasic <ivasic@gmail.com>
	*/
	class BTCORE_EXPORT StatsFile 
	{
	public:
		/**
			* @brief A constructor.
			* Constructs StatsFile object and calls readSync().
			*/
		StatsFile(const QString & filename);
		virtual ~StatsFile();
		
		QString readString(const QString &key);
		
		Uint64 readUint64(const QString &key);
		bool readBoolean(const QString &key);
		int readInt(const QString &key);
		unsigned long readULong(const QString &key);
		float readFloat(const QString &key);
	
		void write(const QString &key, const QString &value);
		
		/// Sync with disk
		void sync();
		
		/**
		 * See if there is a key in the stats file
		 * @param key The key
		 * @return true if key is in the stats file
		 */
		bool hasKey(const QString & key) const;
		
	private:
		KSharedConfigPtr cfg;
	};
}

#endif
