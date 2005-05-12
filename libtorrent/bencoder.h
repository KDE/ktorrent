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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef BTBENCODER_H
#define BTBENCODER_H

#include <qmap.h>
#include <qvaluelist.h>
#include "value.h"


namespace bt
{

	/**
	 * @author Joris Guisson
	 * @brief Helper class to b-encode stuff.
	 * 
	 * This file b-encodes data. For more details about b-encoding, see
	 * the BitTorrent protocol docs.
	 */
	class BEncoder
	{
	public:
		BEncoder();
		virtual ~BEncoder();

		/**
		 * Encode an integer. The integer 14 is encoded as i14e
		 * @param val The integer
		 * @return The integer encoded
		 */
		QString encode(int val);
		
		/**
		 * Encode a string. The string spam is encoded as 4:spam
		 * @param str The string
		 * @return The string encoded
		 */
		QString encode(const QString & str);
		
		/**
		 * Encode a Value. 
		 * @param val The Value
		 * @return The Value encoded
		 */
		QString encode(const Value & val);
		
		/**
		 * Encode a list of values. Lists are encoded this way :
		 * l element1 element2 ...elementN e (ignore spaces)
		 * @param vl The list
		 * @return The list encoded
		 */
		QString encode(const QValueList<Value> & vl);
		
		/**
		 * Encode a dictionary of values. Dicts are encoded this way :
		 * d key1 value1 ... keyN valueN e (ignore spaces)
		 * @param vm The dictionary
		 * @return The dictionary encoded
		 */
		QString encode(const QMap<QString,Value> & vm);
	};

};

#endif
