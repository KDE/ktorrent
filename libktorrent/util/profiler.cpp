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
#ifdef KT_PROFILE
#include <qfile.h>
#include <qtextstream.h>
#include <sys/time.h>
#include "profiler.h"

namespace bt
{
	Profile::Profile(Profile* parent,const QString & name) : parent(parent),name(name)
	{
		min = max = avg = 0.0;
		count = 0;
		start_time = 0.0;
		children.setAutoDelete(true);
	}
	
	Profile::~Profile()
	{
	}
		
	void Profile::start()
	{
		struct timeval tv;
		gettimeofday(&tv,0);
		start_time = tv.tv_sec * 1000.0 + tv.tv_usec * 0.001;
	}
		
	void Profile::end()
	{
		struct timeval tv;
		gettimeofday(&tv,0);
		double end_time = tv.tv_sec * 1000.0 + tv.tv_usec * 0.001;
		double d = end_time - start_time;
		// update stuff
		
		if (d < min || count == 0)
			min = d;
		if (d > max || count == 0)
			max = d;
		
		avg = (avg * count + d) / (count + 1);
		count++;
	}
		
	Profile* Profile::child(const QString & name)
	{
		QPtrList<Profile>::iterator i = children.begin();
		while (i != children.end())
		{
			Profile* p = *i;
			if (p->name == name)
				return p;
			i++;
		}
		
		Profile* p = new Profile(this,name);
		children.append(p);
		return p;
	}
	
	void Profile::save(QTextStream & out,const QString & base)
	{
		QString nb = base + "/" + name;
		
		out.precision(5);
		out << qSetW(60) << nb << qSetW(10) << min << qSetW(10) << max << qSetW(10) << avg << qSetW(10) << count << endl;
		
		QPtrList<Profile>::iterator i = children.begin();
		while (i != children.end())
		{
			Profile* p = *i;
			p->save(out,nb);
			i++;
		} 
	}
	
	/////////////////////
	
	Profiler Profiler::inst;

	Profiler::Profiler() : curr(0),root(0)
	{
		root = new Profile(0,"root");
		curr = root;
	}


	Profiler::~Profiler()
	{
		delete root;
	}

	void Profiler::start(const QString & s)
	{
		curr = curr->child(s);
		curr->start();
	}
	
	void Profiler::end()
	{
		curr->end();
		curr = curr->getParent();
	}

	void Profiler::saveToFile(const QString & fn)
	{
		QFile fptr(fn);
		if (!fptr.open(IO_WriteOnly))
			return;
		
		QTextStream out(&fptr);
		
		out << qSetW(60) << "code" << qSetW(10) << "min" << qSetW(10) << "max" << qSetW(10) << "avg" << qSetW(10) << "count" << endl;
		out << endl; 
		
		root->save(out,QString::null);
	}
}
#endif
