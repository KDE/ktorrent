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
#ifndef BTPTRMAP_H
#define BTPTRMAP_H

#include <map>

namespace bt
{
	/**
	* @author Joris Guisson
	* @brief Map of pointers
	*
	* A Map where the data is a pointer. The PtrMap has an autodeletion feature.
	* When autodelete is on, every time we remove something from the map, the data
	* will be deleted.
	*/
	template <class Key,class Data>
	class PtrMap
	{
		bool autodel;
		std::map<Key,Data*> pmap;
	public:
		/**
		* Constructor.
		* @param auto_del Wether or not to enable auto deletion
		*/
		PtrMap(bool autodel = false) : autodel(autodel)
		{}
		
		/**
		* Destructor. Will delete all objects, if auto deletion is on.
		*/
		virtual ~PtrMap()
		{
			clear();
		}
		
		
		/**
		* Return the number of key data pairs in the map.
		*/
		unsigned int count() const {return pmap.size();}
		
		/**
		* Enable or disable auto deletion.
		* @param yes Enable if true, disable if false
		*/
		void setAutoDelete(bool yes)
		{
			autodel = yes;
		}
		
		typedef typename std::map<Key,Data*>::iterator iterator;
		typedef typename std::map<Key,Data*>::const_iterator const_iterator;
		
		iterator begin() {return pmap.begin();}
		iterator end() {return pmap.end();}
		
		const_iterator begin() const {return pmap.begin();}
		const_iterator end() const {return pmap.end();}
		
		/**
		* Remove all objects, will delete them if autodelete is on.
		*/
		void clear()
		{
			if (autodel)
			{
				for (iterator i = pmap.begin();i != pmap.end();i++)
				{
					delete i->second;
					i->second = 0;
				}
			}
			pmap.clear();
		}
		
		/**
		* Insert a key data pair.
		* @param k The key
		* @param d The data
		* @param overwrite Wether or not to overwrite
		* @return true if the insertion took place
		*/
		bool insert(const Key & k,Data* d,bool overwrite = true)
		{
			iterator itr =  pmap.find(k);
			if (itr != pmap.end())
			{
				if (overwrite)
				{
					if (autodel)
						delete itr->second;
					itr->second = d;
					return true;
				}
				else
				{
					return false;
				}
			}
			else
			{
				pmap[k] = d;
				return true;
			}
		}
	
		/**
		* Find a key in the map and returns it's data.
		* @param k The key
		* @return The data of the key, 0 if the key isn't in the map
		*/
		Data* find(const Key & k)
		{
			iterator i = pmap.find(k);
			return (i == pmap.end()) ? 0 : i->second;
		}
		
		/**
		* Find a key in the map and returns it's data.
		* @param k The key
		* @return The data of the key, 0 if the key isn't in the map
		*/
		const Data* find(const Key & k) const
		{
			const_iterator i = pmap.find(k);
			return (i == pmap.end()) ? 0 : i->second;
		}
		
		/**
		* Check to see if a key is in the map.
		* @param k The key
		* @return true if it is part of the map
		*/
		bool contains(const Key & k) const
		{
			const_iterator i = pmap.find(k);
			return i != pmap.end();
		}
		
		/**
		* Erase a key from the map. Will delete
		* the data if autodelete is on.
		* @param key The key
		* @return true if an erase took place
		*/
		bool erase(const Key & key)
		{
			iterator i = pmap.find(key);
			if (i == pmap.end())
				return false;
			
			if (autodel)
				delete i->second;
			pmap.erase(i);
			return true;
		}
	};

}

#endif
