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
#ifndef BTGARBAGECOLLECTOR_H
#define BTGARBAGECOLLECTOR_H

#include <qobject.h>
#include <qptrlist.h>

namespace bt 
{

	/**
	 * @author Joris Guisson
	 * 
	 * Class to ditch objects nowbody gives a damm about.
	*/
	class GarbageCollector : public QObject
	{
		Q_OBJECT
				
		QPtrList<QObject> garbage;
		bool clearing;
	public:
		GarbageCollector();	
		virtual ~GarbageCollector();
	
		/**
		 * Add an object to the list.
		 * @param obj The object
		 */
		void add(QObject* obj);
		
		/**
		 * Clear all objects.
		 */
		void clear();
		
		/**
		 * Print statistics.
		 */
		void printStats();
	private slots:
		void onDestroyed(QObject* obj);
	};

}

#endif
