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
#ifndef BTSTATSFILE_H
#define BTSTATSFILE_H

#include <qstring.h>
#include <qfile.h>
#include <qmap.h>

#include <util/constants.h>

namespace bt
{

	/**
	 * @brief This class is used for loading/storing torrent stats in a file.
	 * @author Ivan Vasic <ivasic@gmail.com>
	*/
	class StatsFile
	{
		public:
			/**
			 * @brief A constructor.
			 * Constructs StatsFile object and calls readSync().
			 */
			StatsFile(QString filename);
			~StatsFile();
			
			///Closes QFile
			void close();
			
			/**
			 * @brief Main read function.
			 * @return QString value that correspodents to key.
			 * @param key - QString stats key.
			 */
			QString readString(QString key);
			
			Uint64 readUint64(QString key);
			bool readBoolean(QString key);
			int readInt(QString key);
			unsigned long readULong(QString key);
			float readFloat(QString key);
		
			/**
			 * @brief Writes key and value.
			 * It only inserts pair of key/value to the m_values. To make changes to file call writeSync().
			 * @param key - QString key
			 * @param value - QString value.
			 */
			void write(QString key, QString value);
			
			///Reads data from stats file to m_values.
			void readSync();
			
			///Writes data from m_values to stats file.
			void writeSync();
			
			/**
			 * See if there is a key in the stats file
			 * @param key The key
			 * @return true if key is in the stats file
			 */
			bool hasKey(const QString & key) const {return m_values.contains(key);}
			
		private:
			QString m_filename;
			QFile m_file;
			
			QMap<QString, QString> m_values;
	};
}

#endif
