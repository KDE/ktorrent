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
#include "rssfilter.h"

namespace kt
{

	FilterMatch::FilterMatch(int season, int episode, QString link, QString time)
	{
		m_season = season;
		m_episode = episode;
		m_link = link;
		m_time = time;
	}
	
	FilterMatch::FilterMatch(const FilterMatch &other)
	{
		*this = other;
	}
	
	FilterMatch &FilterMatch::operator=(const FilterMatch &other)
	{
		if (&other != this)
		{
			m_season = other.season();
			m_episode = other.episode();
			m_link = other.link();
			m_time = other.time();
		}
		
		return *this;
	}
	
	bool FilterMatch::operator==(const FilterMatch &other) const
	{
		return m_link==other.link() && m_season==other.season() && m_episode==other.episode();
	}
		
	RssFilter::RssFilter(QObject * parent) : QObject(parent)
	{
		m_title = "New";
		m_active = false;
		m_series = false;
		m_sansEpisode = false;
		m_minSeason = m_minEpisode = m_maxSeason = m_maxEpisode = 0;
	}
	
	RssFilter::RssFilter(QString title, bool active, QStringList regExps, bool series, bool sansEpisode, 
					int minSeason, int minEpisode, int maxSeason, int maxEpisode, 
					QValueList<FilterMatch> matches)
	{
		m_title = title;
		m_active = active;
		m_regExps = regExps;
		m_series = series;
		m_sansEpisode = sansEpisode;
		m_minSeason = minSeason;
		m_minEpisode = minEpisode;
		m_maxSeason = maxSeason;
		m_maxEpisode = maxEpisode;
		m_matches = matches;
	}
					
	RssFilter::RssFilter(const RssFilter &other) : QObject()
	{
		*this = other;
	}
	
	RssFilter &RssFilter::operator=(const RssFilter &other)
	{
		if (&other != this)
			{
			m_title = other.title();
			m_active = other.active();
			m_regExps = other.regExps();
			m_series = other.series();
			m_sansEpisode = other.sansEpisode();
			m_minSeason = other.minSeason();
			m_minEpisode = other.minEpisode();
			m_maxSeason = other.maxSeason();
			m_maxEpisode = other.maxEpisode();
			m_matches = other.matches();
		}
		
		return *this;
	}
	
	void RssFilter::setTitle( const QString& title )
	{
		if (m_title != title)
		{
			m_title = title;
			emit titleChanged(title);
		}
	}
	
	void RssFilter::setActive( bool active )
	{
		if (m_active != active)
		{
			m_active = active;
			
			emit activeChanged(active);
		}
	}
	
	void RssFilter::setRegExps( const QStringList& regExps )
	{
		if (regExps != m_regExps)
		{
			m_regExps = regExps;
			
			emit regExpsChanged(regExps);
		}
	}
		
	void RssFilter::setSeries( bool series )
	{
		if (m_series != series)
		{
			m_series = series;
			
			emit seriesChanged(series);
		}
	}
	
	void RssFilter::setSansEpisode( bool sansEpisode )
	{
		if (m_sansEpisode != sansEpisode)
		{
			m_sansEpisode = sansEpisode;
			
			emit sansEpisodeChanged(sansEpisode);
		}
	}
	
	void RssFilter::setMinSeason( int minSeason )
	{
		if (m_minSeason != minSeason)
		{
			m_minSeason = minSeason;
			
			emit minSeasonChanged(minSeason);
		}
	}
	
	void RssFilter::setMinEpisode( int minEpisode )
	{
		if (m_minEpisode != minEpisode)
		{
			m_minEpisode = minEpisode;
			
			emit minEpisodeChanged(minEpisode);
		}
	}
	
	void RssFilter::setMaxSeason( int maxSeason )
	{
		if (m_maxSeason != maxSeason)
		{
			m_maxSeason = maxSeason;
			
			emit maxSeasonChanged(maxSeason);
		}
	}
	
	void RssFilter::setMaxEpisode( int maxEpisode )
	{
		if (m_maxEpisode != maxEpisode)
		{
			m_maxEpisode = maxEpisode;
			
			emit maxEpisodeChanged(maxEpisode);
		}
	}
	
	void RssFilter::setMatches( const QValueList<FilterMatch>& matches )
	{
		if (matches != m_matches)
		{
			m_matches = matches;
			
			emit matchesChanged(matches);
		}
	}
	
	bool RssFilter::episodeInRange(int season, int episode, bool ignoreMatches, bool& alreadyDownloaded)
	{
		if (m_minSeason > 0)
			{
			if (season < m_minSeason)
				{
				return false;
				}
			if (season == m_minSeason && m_minEpisode > 0)
				{
				if (episode < m_minEpisode)
					{
					return false;
					}
				}
			}
			
		if (m_maxSeason > 0)
			{
			if (season > m_maxSeason)
				{
				return false;
				}
			if (season == m_maxSeason && m_maxEpisode > 0)
				{
				if (episode > m_maxEpisode)
					{
					return false;
					}
				}
			}
		
		for (int i=0; i<m_matches.count(); i++)
		{
			//if the episode is already in the matches - don't download it
			if ((*m_matches.at(i)).season() == season &&(*m_matches.at(i)).episode() == episode)
			{
				alreadyDownloaded = true;
				return !ignoreMatches;
			}
		 
		}
		
		return true;
	}
		
	bool RssFilter::scanArticle( RssArticle article, bool ignoreMatches, bool saveMatch)
	{
		if (!m_active && saveMatch)
			return false;
	
		QRegExp regEx;
		regEx.setCaseSensitive(false);
		
		if (!m_regExps.count())
			return false;
		
		for (int i=0; i<m_regExps.count(); i++)
		{
			if (m_regExps[i].isEmpty())
				continue;
				
			QString curExp = m_regExps[i];
			bool invert=false;
			
			if (curExp.startsWith( "!" ))
				{
				invert=true;
				curExp = curExp.remove( 0, 1);
				}
			
			regEx.setPattern(curExp);
			
			if (!invert)
			{
				if (!article.title().contains(regEx) && !article.link().prettyURL().contains(regEx) && !article.description().contains(regEx) )
				{
					return false;
				}
			}
			else
			{
				if (article.title().contains(regEx) || article.link().prettyURL().contains(regEx) || article.description().contains(regEx) )
				{
					return false;
				}
			}
		}
		
		int season = 0, episode = 0;
		bool alreadyDownloaded = false;
		
		if (m_series)
		{
			QStringList episodeFormats;
			episodeFormats << "s([0-9]{1,2})[de]([0-9]{1,2})[^0-9]" << "[^0-9]([0-9]{1,2})x([0-9]{1,2})[^0-9]" << "[^0-9]([0-9]{1,2})([0-9]{2})[^0-9]";
			for (int i=0; i<episodeFormats.count(); i++)
			{
				regEx.setPattern(*episodeFormats.at(i));
				if (regEx.search(article.title()) >= 0)
				{
					season = (*regEx.capturedTexts().at(1)).toInt();
					episode = (*regEx.capturedTexts().at(2)).toInt();
					if (!episodeInRange(season,episode,ignoreMatches,alreadyDownloaded))
					{
						return false;
					}
					break;
				}
			
				if (regEx.search(article.link().prettyURL()) >= 0)
				{
					season = (*regEx.capturedTexts().at(1)).toInt();
					episode = (*regEx.capturedTexts().at(2)).toInt();
					if (!episodeInRange(season,episode,ignoreMatches,alreadyDownloaded))
					{
						return false;
					}
					break;
				}
			
				if (regEx.search(article.description()) >= 0)
				{
					season = (*regEx.capturedTexts().at(1)).toInt();
					episode = (*regEx.capturedTexts().at(2)).toInt();
					if (!episodeInRange(season,episode,ignoreMatches,alreadyDownloaded))
					{
						return false;
					}
					break;
				}
			}
			
			if (!m_sansEpisode)
				{
				if (!season && !episode)
					{
					//no episode number was found and we're not downloading matches without episode numbers
					return false;
					}
				}
		}
		
		if (!alreadyDownloaded && saveMatch)
		{
			FilterMatch newMatch(season, episode, article.link().prettyURL());
			m_matches.append(newMatch);
			emit matchesChanged(m_matches);
		}
		
		return true;
	}
	
	void RssFilter::deleteMatch(const QString& link)
	{
	
		QValueList<FilterMatch>::iterator it = m_matches.begin();
		while (it != m_matches.end())
		{
			if ((*it).link() == link)
			{
				it = m_matches.remove(it);
			}
			else
			{
				it++;
			}
		}
		
	}
	
	QDataStream &operator<<( QDataStream &out, const FilterMatch &filterMatch )
	{
		out << filterMatch.season() << filterMatch.episode() << filterMatch.time() << filterMatch.link();
		
		return out;
	}
	
	QDataStream &operator>>( QDataStream &in, FilterMatch &filterMatch )
	{
		int season, episode;
		QString time;
		QString link;
		in >> season >> episode >> time >> link;
		filterMatch = FilterMatch(season, episode, link, time);
		
		return in;
	}
	
	QDataStream &operator<<( QDataStream &out, const RssFilter &filter )
	{
		out << filter.title() << int(filter.active()) << filter.regExps() << int(filter.series()) << int(filter.sansEpisode()) << filter.minSeason() << filter.minEpisode() << filter.maxSeason() << filter.maxEpisode() << filter.matches();
		
		return out;
	}
	
	QDataStream &operator>>( QDataStream &in, RssFilter &filter )
	{
		QString title;
		int active;
		QStringList regExps;
		int series;
		int sansEpisode;
		int minSeason;
		int minEpisode;
		int maxSeason;
		int maxEpisode;
		QValueList<FilterMatch> matches;
		in >> title >> active >> regExps >> series >> sansEpisode >> minSeason >> minEpisode >> maxSeason >> maxEpisode >> matches;
		
		filter = RssFilter(title, active, regExps, series, sansEpisode, minSeason, minEpisode, maxSeason, maxEpisode, matches);
		
		return in;
	}
	
	RssFilter::~RssFilter()
	{
	}

}
