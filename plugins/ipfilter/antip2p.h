/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#ifndef ANTIP2P_H
#define ANTIP2P_H

#include <util/mmapfile.h>
#include <util/constants.h>

#include <qvaluelist.h>
#include <qstring.h>

namespace kt
{
	typedef struct
	{
		bt::Uint32 ip1;
		bt::Uint32 ip2;
		bt::Uint64 offset;
		bt::Uint32 nrEntries;
	} HeaderBlock;
	
	struct IPBlock
	{
		bt::Uint32 ip1;
		bt::Uint32 ip2;
		
		bool operator < (const IPBlock & b) const;
	};
	
	/**
	 * @author Ivan Vasic <ivasic@gmail.com>
	 * @brief This class is used to manage anti-p2p filter list, so called level1.
	 */
	class AntiP2P
	{
		public:
    		AntiP2P();
    		~AntiP2P();
			
			/**
			 * Checks if anti-p2p file is present. Used to check if we should use level1 list
			 **/
			bool exists();
			
			
			/**
			 * Creates and loads the header from antip2p filter file.
			 **/
			void loadHeader();
			
			
			/**
			 * Checks if specified IP is listed in filter file.
			 * @return TRUE if IP should be blocked, FALSE otherwise
			 * @param ip QString representation of IP to be checked
			 **/
			bool isBlockedIP(const QString& ip);
			
			/**
			 * Overloaded function. Uses Uint32 IP to be checked
			 **/
			bool isBlockedIP(bt::Uint32& ip);
			
			/**
			 * This function converts QString IP to Uint32 format.
			 **/
			static bt::Uint32 toUint32(const QString& ip);
			
		private:
			bt::MMapFile* file;
			QValueList<HeaderBlock> blocks;
			
			///Is AntiP2P header loaded
			bool header_loaded;
			
			/**
			 * Loads filter file
			*/
			void load();
			
			/**
			 * Binary searches AntiP2P::blocks to find range where IP could be.
			 * @returns 
			 * 		-1 if IP cannot be in the list
			 * 		-2 if IP is already found in blocks
			 * 		or index of HeaderBlock in AntiP2P::blocks which will be used for direct file search.
			 **/
			int searchHeader(bt::Uint32& ip, int start, int end);
			
			
			/**
			 * Binary searches AntiP2P::file to find IP.
			 * @returns TRUE if IP should be blocked FALSE otherwise
			 **/
			bool searchFile(IPBlock* file_blocks, bt::Uint32& ip, int start, int end);
			
	};
}
#endif
