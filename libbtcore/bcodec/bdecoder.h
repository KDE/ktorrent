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
#ifndef BTBDECODER_H
#define BTBDECODER_H

#include <qstring.h>
#include <util/constants.h>
#include <btcore_export.h>

namespace bt
{

	class BNode;
	class BListNode;
	class BDictNode;
	class BValueNode;
	
	/**
	 * @author Joris Guisson
	 * @brief Decodes b-encoded data
	 *
	 * Class to decode b-encoded data.
	 */
	class BTCORE_EXPORT BDecoder
	{
		const QByteArray & data;
		Uint32 pos;
		bool verbose;
		int level;
	public:
		/**
		 * Constructor, passes in the data to decode.
		 * @param data The data
		 * @param verbose Verbose output to the log
		 * @param off Offset to start parsing
		 */
		BDecoder(const QByteArray & data,bool verbose,Uint32 off = 0);
		virtual ~BDecoder();

		/**
		 * Decode the data, the root node gets
		 * returned. (Note that the caller must delete this node)
		 * @return The root node
		 */
		BNode* decode();
		
		/**
		 * Decode the data, the root list node gets
		 * returned. (Note that the caller must delete this node)
		 * @return The root node
		 */
		BListNode* decodeList();
		
		/**
		 * Decode the data, the root dict node gets
		 * returned. (Note that the caller must delete this node)
		 * @return The root node
		 */
		BDictNode* decodeDict();
		
		/// Get the current position in the data
		Uint32 position() const {return pos;}
	private:
		void debugMsg(const QString & msg);
		
	private:
		BDictNode* parseDict();
		BListNode* parseList();
		BValueNode* parseInt();
		BValueNode* parseString();
	};

}

#endif
