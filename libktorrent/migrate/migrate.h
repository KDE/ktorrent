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
#ifndef BTMIGRATE_H
#define BTMIGRATE_H

namespace bt
{
	class Torrent;

	/**
		@author Joris Guisson <joris.guisson@gmail.com>
	
		Class to migrate old pre-mmap downloads to new ones
	*/
	class Migrate
	{
	public:
		Migrate();
		virtual ~Migrate();

		/**
		 * Migrate a download to the new format.
		 * @param tor The torrent
		 * @param tor_dir TorX directory
		 * @param sdir The save directory
		 * @throw Error if something goes wrong
		 */
		void migrate(const Torrent & tor,const QString & tor_dir,const QString & sdir);
	private:
		bool preMMap(const QString & current_chunks);
		void migrateCurrentChunks(const QString & current_chunks);
	};

}

#endif
