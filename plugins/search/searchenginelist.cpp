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
#include <qfile.h>
#include <qtextstream.h>
#include <qstringlist.h>
#include "searchenginelist.h"

using namespace bt;

namespace kt
{

	SearchEngineList::SearchEngineList()
	{}


	SearchEngineList::~SearchEngineList()
	{}

	void SearchEngineList::save(const QString& file)
	{
		QFile fptr(file);
		if (!fptr.open(IO_WriteOnly))
			return;
		
		QTextStream out(&fptr);
		out << "# PLEASE DO NOT MODIFY THIS FILE. Use KTorrent configuration dialog for adding new search engines." << ::endl;
		out << "# SEARCH ENGINES list" << ::endl;
     
		QValueList<SearchEngine>::iterator i = m_search_engines.begin();
		while (i != m_search_engines.end())
		{
			SearchEngine & e = *i;

			// replace spaces by %20
			QString name = e.name;
			name = name.replace(" ","%20");
			QString u = e.url.prettyURL();
			u = u.replace(" ","%20");
			out << name << " " << u << ::endl;
			i++;
		}
	}
	
	void SearchEngineList::load(const QString& file)
	{
		m_search_engines.clear();
		
		QFile fptr(file);
		
		if(!fptr.exists())
			makeDefaultFile(file);
		
		if (!fptr.open(IO_ReadOnly))
			return;
		
		QTextStream in(&fptr);
		
		int id = 0;
		
		while (!in.atEnd())
		{
			QString line = in.readLine();
		
			if(line.startsWith("#") || line.startsWith(" ") || line.isEmpty() ) continue;
		
			QStringList tokens = QStringList::split(" ", line);
		
			SearchEngine se;
			se.name = tokens[0];
			se.name = se.name.replace("%20"," ");
			se.url = KURL::fromPathOrURL(tokens[1]);
		
			for(Uint32 i=2; i<tokens.count(); ++i)
				se.url.addQueryItem(tokens[i].section("=",0,0), tokens[i].section("=", 1, 1));
		
			m_search_engines.append(se);
		}
	}
	
	void SearchEngineList::makeDefaultFile(const QString& file)
	{
		QFile fptr(file);
		if (!fptr.open(IO_WriteOnly))
			return;
		
		QTextStream out(&fptr);
		out << "# PLEASE DO NOT MODIFY THIS FILE. Use KTorrent configuration dialog for adding new search engines." << ::endl;
		out << "# SEARCH ENGINES list" << ::endl;
		out << "KTorrents http://www.ktorrents.com/search.php?lg=0&sourceid=ktorrent&q=FOOBAR&f=0" << ::endl;
		out << "bittorrent.com http://www.bittorrent.com/search_result.myt?search=FOOBAR" << ::endl; 
		out << "isohunt.com http://isohunt.com/torrents.php?ihq=FOOBAR&op=and" << ::endl; 
		out << "mininova.org http://www.mininova.org/search.php?search=FOOBAR" << ::endl; 
		out << "thepiratebay.org http://thepiratebay.org/search.php?q=FOOBAR" << ::endl; 
		out << "bitoogle.com http://bitoogle.com/search.php?q=FOOBAR" << ::endl; 
		out << "bytenova.org http://www.bitenova.org/search.php?search=FOOBAR&start=0&start=0&ie=utf-8&oe=utf-8" << ::endl; 
		out << "torrentspy.com http://torrentspy.com/search.asp?query=FOOBAR" << ::endl; 
		out << "torrentz.com http://www.torrentz.com/search_FOOBAR" << ::endl; 
	}
		
	KURL SearchEngineList::getSearchURL(bt::Uint32 engine) const
	{
		if (engine >= m_search_engines.count())
			return KURL();
		else
			return m_search_engines[engine].url;
	}
	
	QString SearchEngineList::getEngineName(bt::Uint32 engine) const
	{
		if (engine >= m_search_engines.count())
			return QString::null;
		else
			return m_search_engines[engine].name;
	}

}
