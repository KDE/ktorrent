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
#ifndef BTARRAY_H
#define BTARRAY_H

#include "constants.h"

namespace bt
{

	/**
	 * @author Joris Guisson
	 *
	 * Template array classes, makes creating dynamic buffers easier
	 * and safer.
	 */
	template<class T>
	class Array
	{
		Uint32 num;
		T* data;
	public:
		Array(Uint32 num = 0) : num(num),data(0)
		{
			if (num > 0)
				data = new T[num];
		}

		~Array()
		{
			delete [] data;
		}

		T & operator [] (Uint32 i) {return data[i];}
		const T & operator [] (Uint32 i) const {return data[i];}

		operator const T* () const {return data;}
		operator T* () {return data;}

		/// Get the number of elements in the array
		Uint32 size() const {return num;}

		/**
		 * Fill the array with a value
		 * @param val The value
		 */
		void fill(T val)
		{
			for (Uint32 i = 0;i < num;i++)
				data[i] = val;
		}
	};

}

#endif
