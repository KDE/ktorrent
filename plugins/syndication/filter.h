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
#ifndef KTFILTER_H
#define KTFILTER_H


#include <QList>
#include <QRegExp>
#include <syndication/item.h>

namespace bt
{
	class BEncoder;
	class BDictNode;
}

namespace kt
{
	

	/**
		Class to filter torrents from Feeds
	*/
	class Filter
	{
	public:
		Filter();
		Filter(const QString & name);
		virtual ~Filter();
		
		/**
		 * Start matching items, must be called before the first call to match, when matching
		 * a list of Syndication::Item's.
		 */
		void startMatching();
		
		/// Get the name of the filter
		const QString & filterName() const {return name;}
		
		/// Get the filter ID
		const QString & filterID() const {return id;}
		
		/// Set the name of the filter
		void setFilterName(const QString & n) {name = n;}
		
		/**
		 * Check if an item matches the filter
		 * @param item The item
		 * @return true If a match is found, false otherwise
		 */
		bool match(Syndication::ItemPtr item);
		
		/// Add a word match
		void addWordMatch(const QRegExp & exp);
		
		/// Remove a word match
		void removeWordMatch(const QRegExp & exp);
		
		/// Get all word matches
		QList<QRegExp> wordMatches() const {return word_matches;}
		
		/// Clear the list of word matches
		void clearWordMatches() {word_matches.clear();}
		
		/// Is season and episode matching enabled
		bool useSeasonAndEpisodeMatching() const {return use_season_and_episode_matching;}
		
		/// Enable or disable season and episode matching
		void setSeasonAndEpisodeMatching(bool on) {use_season_and_episode_matching = on;}
		
		/// Do not download duplicate season and episode matches
		bool noDuplicateSeasonAndEpisodeMatches() const {return no_duplicate_se_matches;}
		
		/// Set whether or not to download duplicate season and episode matches
		void setNoDuplicateSeasonAndEpisodeMatches(bool on) {no_duplicate_se_matches = on;}
		
		/// Get the seasons to download represented in a string
		QString seasonsToString() const {return seasons_string;}
		
		/// Get the episodes to download represented in a string
		QString episodesToString() const {return episodes_string;}
		
		/**
		 * Set the seasons from a string
		 * @param s The string
		 * @return true if string is properly formatted
		 */
		bool setSeasons(const QString & s);
		
		/**
		 * Set the episodes from a string
		 * @param s The string
		 * @return true if string is properly formatted
		 */
		bool setEpisodes(const QString & s);
		
		/// Download the matching items or not
		bool downloadMatching() const {return download_matching;}
		
		/// Set the download matching or not
		void setDownloadMatching(bool on) {download_matching = on;}
		
		/// Download the non matching items or not
		bool downloadNonMatching() const {return download_non_matching;}
		
		/// Set the download non matching or not
		void setDownloadNonMatching(bool on) {download_non_matching = on;}
		
		/// Get the group
		const QString & group() const {return dest_group;}
		
		/// Set the group 
		void setGroup(const QString & g) {dest_group = g;}
		
		/// Get the download location
		const QString & downloadLocation() const  {return download_location;}
		
		/// Set the download location
		void setDownloadLocation(const QString & dl) {download_location = dl;}
		
		/// Open torrents silently or not
		bool openSilently() const {return silent;}
		
		/// Enable or disable open silently
		void setOpenSilently(bool on) {silent = on;}
		
		/// Are the word matches case sensitive
		bool caseSensitive() const {return case_sensitive;}
		
		/// Set case sensitivity of word matches
		void setCaseSensitive(bool on) {case_sensitive = on;}
		
		/// Return wether or not all word matches must match
		bool allWordMatchesMustMatch() const {return all_word_matches_must_match;}
		
		/// Set wether or not all word matches must match
		void setAllWordMatchesMustMatch(bool on) {all_word_matches_must_match = on;}
		
		/// Save the filter
		void save(bt::BEncoder & enc);
		
		/// Load the filter
		bool load(bt::BDictNode* dict);
		
		/// Wether or not the string matches are regular expressions
		bool useRegularExpressions() const {return use_regular_expressions;}
		
		/// Enable or disable regular expressions
		void setUseRegularExpressions(bool on) {use_regular_expressions = on;}
		
		/// Is a string a valid seasons or episode string
		static bool validSeasonOrEpisodeString(const QString & s);
	private:
		struct Range
		{
			int start;
			int end;
		};
		
		struct MatchedSeasonAndEpisode
		{
			int season;
			int episode;
			
			bool operator == (const MatchedSeasonAndEpisode & se) const
			{
				return season == se.season && episode == se.episode;
			}
		};
		
		static bool parseNumbersString(const QString & s,QList<Range> & numbers);
		static bool stringToRange(const QString & s,Range & r);
		
		bool match(const QString & title,QRegExp & exp);
		bool getSeasonAndEpisode(const QString & title,int & season,int & episode);
		
	private:
		QString id;
		QString name;
		QList<QRegExp> word_matches;
		bool use_season_and_episode_matching;
		bool no_duplicate_se_matches;
		QList<Range> seasons;
		QString seasons_string;
		QList<Range> episodes;
		QString episodes_string;
		bool download_matching;
		bool download_non_matching;
		QString dest_group;
		QString download_location;
		bool silent;
		bool case_sensitive;
		bool all_word_matches_must_match;
		bool use_regular_expressions;
		
		QList<MatchedSeasonAndEpisode> se_matches;
	};

}

#endif
