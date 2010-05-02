/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
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
#include <util/log.h>
#include <util/sha1hash.h>
#include <bcodec/bencoder.h>
#include <bcodec/bnode.h>
#include "filter.h"

using namespace bt;

namespace kt
{
	static QString RandomID()
	{
		Uint8 data[20];
		qsrand(time(0));
		for (int i = 0;i < 20;i++)
			data[i] = qrand();
		return QString("filter:%1").arg(SHA1Hash::generate(data,20).toString());
	}
	
	Filter::Filter()
	{
		id = RandomID();
		use_season_and_episode_matching = false;
		download_matching = true;
		download_non_matching = false;
		silent = true;
		case_sensitive = false;
		all_word_matches_must_match = false;
		use_regular_expressions = false;
		no_duplicate_se_matches = true;
	}

	Filter::Filter(const QString & name) : name(name)
	{
		id = RandomID();
		use_season_and_episode_matching = false;
		download_matching = true;
		download_non_matching = false;
		silent = true;
		case_sensitive = false;
		all_word_matches_must_match = false;
		use_regular_expressions = false;
		no_duplicate_se_matches = true;
	}


	Filter::~Filter()
	{
	}
	
	bool Filter::getSeasonAndEpisode(const QString & title,int & season,int & episode)
	{
		QStringList se_formats;
		se_formats << "(\\d+)x(\\d+)"
				<< "S(\\d+)E(\\d+)"
				<< "(\\d+)\\.(\\d+)"
				<< "S(\\d+)\\.E(\\d+)";
		
		foreach (const QString & format,se_formats)
		{
			QRegExp exp(format,Qt::CaseInsensitive);
			int pos = exp.indexIn(title);
			if (pos > -1) 
			{
				QString s = exp.cap(1); // Season
				QString e = exp.cap(2);  // Episode
				bool ok = false;
				season = s.toInt(&ok);
				if (!ok)
					continue;
				
				episode = e.toInt(&ok);
				if (!ok)
					continue;
				
				return true;
			}
		}
		
		return false;
	}
	
	bool Filter::match(const QString & title,QRegExp & exp)
	{
		int pos = 0;
		return ((pos = exp.indexIn(title, pos)) != -1);
	}

	bool Filter::match(Syndication::ItemPtr item)
	{
		bool found_match = false;
		foreach (const QRegExp & exp,word_matches)
		{
			QRegExp tmp = exp;
			tmp.setCaseSensitivity(case_sensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
			tmp.setPatternSyntax(use_regular_expressions ? QRegExp::RegExp : QRegExp::Wildcard);
			if (all_word_matches_must_match)
			{
				if (!match(item->title(),tmp))
					return false;
				else
					found_match = true;
			}
			else if (match(item->title(),tmp))
			{
				found_match = true;
				break; 
			}
		}
		
		if (!found_match)
			return false;
		
		if (use_season_and_episode_matching)
		{
			int season = 0;
			int episode = 0;
			if (!getSeasonAndEpisode(item->title(),season,episode))
				return false;
			
			bool found = false;
			foreach (const Range & r,seasons)
			{
				if (season >= r.start && season <= r.end)
				{
					found = true;
					break;
				}
			}
			
			if (!found)
				return false;
			
			found = false;
			foreach (const Range & r,episodes)
			{
				if (episode >= r.start && episode <= r.end)
				{
					found = true;
					break;
				}
			}
			
			if (!found)
				return false;
			
			if (no_duplicate_se_matches)
			{
				MatchedSeasonAndEpisode se = {season,episode};
				if (se_matches.contains(se))
					return false;
				
				se_matches.append(se);
			}
		}
		
		return true;
	}
	
	void Filter::addWordMatch(const QRegExp & exp)
	{
		word_matches.append(exp);
	}
		
	void Filter::removeWordMatch(const QRegExp & exp)
	{
		word_matches.removeAll(exp);
	}
	
	bool Filter::stringToRange(const QString & s,Range & r)
	{
		QString tmp = s.trimmed(); // Get rid of whitespace
		if (tmp.contains("-"))
		{
			// It's a range
			QStringList parts = s.split("-");
			if (parts.count() != 2)
				return false;
			
			bool ok = false;
			int start = parts[0].trimmed().toInt(&ok);
			if (!ok || start < 0)
				return false;
			
			ok = false;
			int end = parts[1].trimmed().toInt(&ok);
			if (!ok || end < 0)
				return false;
			
			r.start = start;
			r.end = end;
		}
		else
		{
			// It's a number
			bool ok = false;
			int num = tmp.toInt(&ok);
			if (!ok || num < 0)
				return false;
			
			r.start = r.end = num;
		}
		
		return true;
	}
	
	bool Filter::parseNumbersString(const QString & s,QList<Range> & numbers)
	{
		QList<Range> results;
		QStringList parts = s.split(",");
		foreach (const QString & p,parts)
		{
			Range r = {0,0};
			if (stringToRange(p,r))
				results.append(r);
			else
				return false;
		}
		
		numbers.clear();
		numbers = results;
		return true;
	}
	
	bool Filter::validSeasonOrEpisodeString(const QString & s)
	{
		QList<Range> tmp;
		return Filter::parseNumbersString(s,tmp);
	}
	
	bool Filter::setSeasons(const QString & s)
	{
		if (parseNumbersString(s,seasons))
		{
			seasons_string = s;
			return true;
		}
		return false;
	}

	bool Filter::setEpisodes(const QString & s)
	{
		if (parseNumbersString(s,episodes))
		{
			episodes_string = s;
			return true;
		}
		return false;
	}
	
	void Filter::save(bt::BEncoder & enc)
	{
		enc.beginDict();
		enc.write("id",id);
		enc.write("name",name);
		enc.write("case_sensitive",case_sensitive);
		enc.write("all_word_matches_must_match",all_word_matches_must_match);
		enc.write("word_matches"); 
		enc.beginList();
		foreach (const QRegExp & exp,word_matches)
			enc.write(exp.pattern());
		enc.end();
		enc.write("use_season_and_episode_matching",use_season_and_episode_matching);
		enc.write("no_duplicate_se_matches",no_duplicate_se_matches);
		enc.write("seasons",seasons_string);
		enc.write("episodes",episodes_string);
		enc.write("download_matching",download_matching);
		enc.write("download_non_matching",download_non_matching);
		if (!dest_group.isEmpty())
			enc.write("group",dest_group);
		if (!download_location.isEmpty())
			enc.write("download_location",download_location);
		enc.write("silently",silent);
		enc.write("use_regular_expressions",use_regular_expressions);
		enc.end();
	}
	
	bool Filter::load(bt::BDictNode* dict)
	{
		BValueNode* vn = dict->getValue("name");
		if (!vn)
			return false;
		
		name = vn->data().toString();
		
		vn = dict->getValue("id");
		if (vn)
			id = vn->data().toString();
		
		vn = dict->getValue("case_sensitive");
		if (!vn)
			return false;
		
		case_sensitive = vn->data().toInt() == 1;
		
		vn = dict->getValue("all_word_matches_must_match");
		if (!vn)
			return false;
		
		all_word_matches_must_match = vn->data().toInt() == 1;
		
		BListNode* ln = dict->getList("word_matches");
		if (!ln)
			return false;
		
		for (Uint32 i = 0;i < ln->getNumChildren();i++)
		{
			vn = ln->getValue(i);
			if (vn)
				word_matches.append(QRegExp(vn->data().toString(),case_sensitive ? Qt::CaseSensitive : Qt::CaseInsensitive));
		}
		
		vn = dict->getValue("use_season_and_episode_matching");
		if (!vn)
			return false;
		
		use_season_and_episode_matching = vn->data().toInt() == 1;
		
		vn = dict->getValue("no_duplicate_se_matches");
		if (vn)
			no_duplicate_se_matches = vn->data().toInt() == 1;
		else
			no_duplicate_se_matches = true;
		
		vn = dict->getValue("seasons");
		if (!vn)
			return false;
		
		setSeasons(vn->data().toString());
		
		vn = dict->getValue("episodes");
		if (!vn)
			return false;
		
		setEpisodes(vn->data().toString());
		
		vn = dict->getValue("download_matching");
		if (!vn)
			return false;
		
		download_matching = vn->data().toInt() == 1;
		 
		vn = dict->getValue("download_non_matching");
		if (!vn)
			return false;
		
		download_non_matching = vn->data().toInt() == 1;
		
		vn = dict->getValue("group");
		if (vn)
			setGroup(vn->data().toString());
		
		vn = dict->getValue("download_location");
		if (vn)
			setDownloadLocation(vn->data().toString());
		
		vn = dict->getValue("silently");
		if (!vn)
			return false;
		
		silent = vn->data().toInt() == 1;
		
		vn = dict->getValue("use_regular_expressions");
		if (vn)
			use_regular_expressions = vn->data().toInt() == 1;
		
		return true;
	}
	
	void Filter::startMatching()
	{
		se_matches.clear();
	}
}
