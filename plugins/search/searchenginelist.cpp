/***************************************************************************
 *   Copyright (C) 2005-2007 by Joris Guisson                              *
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
	{
		m_default_list.append(SearchEngine("KTorrents",KUrl("http://www.ktorrents.com/search.php?lg=0&sourceid=ktorrent&q=FOOBAR&f=0")));
		m_default_list.append(SearchEngine("bittorrent.com",KUrl("http://www.bittorrent.com/search_result.myt?search=FOOBAR")));
		m_default_list.append(SearchEngine("mininova.org",KUrl("http://www.mininova.org/search.php?search=FOOBAR")));
		m_default_list.append(SearchEngine("isohunt.com",KUrl("http://isohunt.com/torrents.php?ihq=FOOBAR&op=and")));
		m_default_list.append(SearchEngine("thepiratebay.org",KUrl("http://thepiratebay.org/search/FOOBAR")));
		m_default_list.append(SearchEngine("bitoogle.com",KUrl("http://bitoogle.com/search.php?q=FOOBAR")));
		m_default_list.append(SearchEngine("bytenova.org",KUrl("http://www.bitenova.org/search.php?search=FOOBAR&start=0&start=0&ie=utf-8&oe=utf-8")));
		m_default_list.append(SearchEngine("torrentz.com",KUrl("http://www.torrentz.com/search_FOOBAR")));
		m_default_list.append(SearchEngine("btjunkie.org",KUrl("http://btjunkie.org/search?q=FOOBAR")));
	}


	SearchEngineList::~SearchEngineList()
	{}

	void SearchEngineList::save(const QString& file)
	{
		QFile fptr(file);
		if (!fptr.open(QIODevice::WriteOnly))
			return;
		
		QTextStream out(&fptr);
		out << "# PLEASE DO NOT MODIFY THIS FILE. Use KTorrent configuration dialog for adding new search engines." << ::endl;
		out << "# SEARCH ENGINES list" << ::endl;
     
		foreach (const SearchEngine & e,m_search_engines)
		{
			// replace spaces by %20
			QString name = e.name;
			name = name.replace(" ","%20");
			QString u = e.url.prettyUrl();
			u = u.replace(" ","%20");
			out << name << " " << u << ::endl;
		}
	}
	
	void SearchEngineList::load(const QString& file)
	{
		m_search_engines.clear();
		
		QFile fptr(file);
		
		if(!fptr.exists())
			makeDefaultFile(file);
		
		if (!fptr.open(QIODevice::ReadOnly))
			return;
		
		QTextStream in(&fptr);
		
		while (!in.atEnd())
		{
			QString line = in.readLine();
		
			if(line.startsWith("#") || line.startsWith(" ") || line.isEmpty() ) continue;
		
			QStringList tokens = line.split(" ");
		
			SearchEngine se;
			se.name = tokens[0];
			se.name = se.name.replace("%20"," ");
			se.url = KUrl(tokens[1]);
		
			for (Uint32 i=2; i < (Uint32)tokens.count(); ++i)
				se.url.addQueryItem(tokens[i].section("=",0,0), tokens[i].section("=", 1, 1));
		
			// check if we need to update the URL of a default item
			foreach (const SearchEngine & e,m_default_list)
			{
				if (e.name == se.name)
				{
					if (e.url != se.url)
						se.url = e.url;
					break;
				}
			}
			
			m_search_engines.append(se);
		}
	}
	
	void SearchEngineList::makeDefaultFile(const QString& file)
	{
		QFile fptr(file);
		if (!fptr.open(QIODevice::WriteOnly))
			return;
		
		QTextStream out(&fptr);
		out << "# PLEASE DO NOT MODIFY THIS FILE. Use KTorrent configuration dialog for adding new search engines." << ::endl;
		out << "# SEARCH ENGINES list" << ::endl;
		foreach (const SearchEngine & e,m_default_list)
		{
			out << e.name << " " << e.url.prettyUrl() << endl;
		}
	}
		
	KUrl SearchEngineList::getSearchURL(bt::Uint32 engine) const
	{
		if (engine >= (Uint32)m_search_engines.count())
			return KUrl();
		else
			return m_search_engines[engine].url;
	}
	
	QString SearchEngineList::getEngineName(bt::Uint32 engine) const
	{
		if (engine >= (Uint32)m_search_engines.count())
			return QString::null;
		else
			return m_search_engines[engine].name;
	}

}
