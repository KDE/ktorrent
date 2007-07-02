/***************************************************************************
 *   Copyright (C) 2006 by Alan Jones                                      *
 *   skyphyr@gmail.com                                                     *
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
#include "rssarticle.h"

namespace kt
{

	RssArticle::RssArticle( )
		{
		
		}
	
	RssArticle::RssArticle(RSS::Article article)
		{
			//these lines generate errors when compiling
			m_title = article.title();
			m_link = article.link();
			m_description = article.description();
			m_pubDate = article.pubDate();
			m_guid = article.guid();
			m_downloaded = 0;
		}
	
	RssArticle::RssArticle(const RssArticle &other)
		{
			*this = other;
		}
	
	RssArticle::RssArticle(QString title, KURL link, QString description, QDateTime pubDate, QString guid, int downloaded)
		{
			m_title = title;
			m_link = link;
			m_description = description;
			m_pubDate = pubDate;
			m_guid = guid;
			m_downloaded = downloaded;
		}
	
	RssArticle &RssArticle::operator=(const RssArticle &other)
	{
		if (&other != this)
		{
			m_title = other.title();
			m_link = other.link();
			m_description = other.description();
			m_pubDate = other.pubDate();
			m_guid = other.guid();
			m_downloaded = other.downloaded();
		}
		return *this;
	}
	
	bool RssArticle::operator==(const RssArticle &other) const
	{
		//let's try just using the guid for now as it should be sufficient
		//return m_title==other.title() && m_link==other.link() && m_description==other.description() && m_pubDate==other.pubDate();
		return m_guid==other.guid();
	}
	
	QDataStream &operator<<( QDataStream &out, const RssArticle &article )
		{
		out << article.title() << article.link() << article.description() << article.pubDate() << article.guid() << article.downloaded();
		
		return out;
		}
	
	QDataStream &operator>>( QDataStream &in, RssArticle &article )
		{
		KURL link;
		QString title;
		QString description;
		QDateTime pubDate;
		QString guid;
		int downloaded;
		in >> title >> link >> description >> pubDate >> guid >> downloaded;
		article = RssArticle(title, link, description, pubDate, guid, downloaded);
		
		return in;
		}
	
	RssArticle::~RssArticle()
		{
		
		}

}
