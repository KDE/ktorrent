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
#ifndef BTPTRLIST_H
#define BTPTRLIST_H

#include <list>

namespace bt
{

	/**
	@author Joris Guisson
	*/
	template<class T>
	class PtrList
	{
		std::list<T*> plist;
		bool autodel;
	public:
		PtrList() : autodel(false) {}
		virtual ~PtrList() {clear();}
		
		typedef typename std::list<T*>::iterator iterator;
		typedef typename std::list<T*>::const_iterator const_iterator;

		iterator begin() {return plist.begin();}
		iterator end() {return plist.end();}
		
		const_iterator begin() const {return plist.begin();}
		const_iterator end() const {return plist.end();}
		
		void setAutoDelete(bool on) {autodel = on;}
		
		void clear()
		{
			if (autodel)
				for (iterator i = plist.begin();i != plist.end();i++)
					delete *i;
			
			plist.clear();
		}
		
		iterator find(T* ptr)
		{
			iterator i = begin();
			while (i != end() && *i != ptr)
				i++;
			
			return i;
		}
		
		const_iterator find(T* ptr) const
		{
			const_iterator i = begin();
			while (i != end() && *i != ptr)
				i++;
			
			return i;
		}

		T* front()
		{
			if (plist.empty())
				return 0;
			else
				return plist.front();
		}

		const T* front() const
		{
			if (plist.empty())
				return 0;
			else
				return plist.front();
		}
		
		void append(T* ptr)
		{
			plist.push_back(ptr);
		}
		
		void erase(T* ptr)
		{
			iterator i = begin();
			while (i != end())
			{
				if (*i == ptr)
				{
					i = plist.erase(i);
				}
				else
				{
					i++;
				}
			}
			if (autodel)
				delete ptr;
		}
		
		T* at(unsigned int idx)
		{
			unsigned int c = 0;
			iterator i = begin();
			while (i != end() && c < idx)
			{
				i++; c++;
			}
			if (i == end())
				return 0;
			else
				return *i;
		}
		
		const T* at(unsigned int idx) const
		{
			unsigned int c = 0;
			const_iterator i = begin();
			while (i != end() && c < idx)
			{
				i++; c++;
			}
			if (i == end())
				return 0;
			else
				return *i;
		}

		void eraseFirst()
		{
			if (plist.empty())
				return;

			T* d = plist.front();
			plist.pop_front();
			if (autodel)
				delete d;
		}
		
		unsigned int count() const {return plist.size();}
	};

}

#endif
