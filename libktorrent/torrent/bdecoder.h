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
#ifndef BTBDECODER_H
#define BTBDECODER_H

#include <qstring.h>

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
	class BDecoder
	{
		const QByteArray & data;
		unsigned int pos;
		bool verbose;
	public:
		/**
		 * Constructor, passes in the data to decode.
		 * @param data The data
		 * @param verbose Verbose output to the log
		 */
		BDecoder(const QByteArray & data,bool verbose);
		virtual ~BDecoder();

		/**
		 * Decode the data, the root node gets
		 * returned. (Note that the caller must delete this node)
		 * @return The root node
		 */
		BNode* decode();
	private:
		BDictNode* parseDict();
		BListNode* parseList();
		BValueNode* parseInt();
		BValueNode* parseString();
	};

}

#endif
