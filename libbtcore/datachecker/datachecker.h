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
#ifndef BTDATACHECKER_H
#define BTDATACHECKER_H

#include <util/bitset.h>
#include <btcore_export.h>
#include "datacheckerlistener.h"

class QString;


namespace bt 
{
	class Torrent;
	

	/**
	 * @author Joris Guisson
	 * 
	 * Checks which data is downloaded, given a torrent and a file or directory containing
	 * files of the torrent.
	*/
	class BTCORE_EXPORT DataChecker
	{
	public:
		DataChecker();	
		virtual ~DataChecker();
	
		/// Set the listener
		void setListener(DataCheckerListener* l) {listener = l;}
		
		/**
		 * Check to see which chunks have been downloaded of a torrent, and which chunks fail.
		 * The corresponding bitsets should be filled with this information.
		 * If anything goes wrong and Error should be thrown. 
		 * @param path path to the file or dir (this needs to end with the name suggestion of the torrent)
		 * @param tor The torrent
		 * @param dnddir DND dir, optional argument if we know this
		 * @param status Current status of the torrent
		 */
		virtual void check(const QString & path,const Torrent & tor,const QString & dnddir,const BitSet & status) = 0;
		
		/**
		 * Get the BitSet representing all the downloaded chunks and which is the result of the data check.
		 */
		const BitSet & getResult() const {return result;}
		
		/// Get the listener
		DataCheckerListener* getListener() {return listener;}
	protected:
		BitSet result;
		Uint32 failed,found,downloaded,not_downloaded;
		DataCheckerListener* listener;
	};

}

#endif
