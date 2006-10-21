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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#ifndef BTPROFILER_H
#define BTPROFILER_H

#ifdef KT_PROFILE
#include <qptrlist.h>
#include <util/constants.h>

class QTextStream;


namespace bt
{
	/**
	 * Profile of one function or section of code.
	 */
	class Profile
	{
		Profile* parent;
		QPtrList<Profile> children;
		
		QString name;
		double min,max,avg;
		Uint32 count;
		double start_time;
	public:	
		Profile(Profile* parent,const QString & name);
		virtual ~Profile();
		
		/**
		 * We just entered the function and will profile it.
		 */
		void start();
		
		/**
		 * We just left the function, internal variables will now be updated
		 */
		void end();
		
		/**
		 * Get a child, if it doesn't exist it will be created.
		 * @param name The name of the child
		 * @return The child
		 */
		Profile* child(const QString & name);
		
		/**
		 * Get the parent of the current profile.
		 */
		Profile* getParent() const {return parent;}
		
		/**
		 * Save profile information to a file.
		 * @param out Text stream to write to
		 * @param base Base path of the profiles
		 */
		void save(QTextStream & out,const QString & base);
	};

	/**
	 * @author Joris Guisson <joris.guisson@gmail.com>
	 * 
	 * Class used to profile ktorrent
	*/
	class Profiler
	{
		Profile* curr;
		Profile* root;
				
		static Profiler inst;
		
		Profiler();
	public:
		virtual ~Profiler();

		void start(const QString & s);
		void end();
		void saveToFile(const QString & fn);
		
		static Profiler & instance() {return inst;} 
	};
}
#define KT_PROF_START(S) bt::Profiler::instance().start(S)
#define KT_PROF_END()  bt::Profiler::instance().end()
#else
#define KT_PROF_START(S)
#define KT_PROF_END()
#endif

#endif
