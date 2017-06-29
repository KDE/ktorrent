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

#include <QTextCodec>

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
        for (int i = 0; i < 20; i++)
            data[i] = qrand();
        return QStringLiteral("filter:%1").arg(SHA1Hash::generate(data, 20).toString());
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
        exclusion_case_sensitive = false;
        exclusion_all_must_match = false;
        exclusion_reg_exp = false;
        no_duplicate_se_matches = true;
    }

    Filter::Filter(const QString& name) : name(name)
    {
        id = RandomID();
        use_season_and_episode_matching = false;
        download_matching = true;
        download_non_matching = false;
        silent = true;
        case_sensitive = false;
        all_word_matches_must_match = false;
        use_regular_expressions = false;
        exclusion_case_sensitive = false;
        exclusion_all_must_match = false;
        exclusion_reg_exp = false;
        no_duplicate_se_matches = true;
    }


    Filter::~Filter()
    {
    }

    bool Filter::getSeasonAndEpisode(const QString& title, int& season, int& episode)
    {
        QStringList se_formats;
        se_formats << QStringLiteral("(\\d+)x(\\d+)")
                   << QStringLiteral("S(\\d+)E(\\d+)")
                   << QStringLiteral("(\\d+)\\.(\\d+)")
                   << QStringLiteral("S(\\d+)\\.E(\\d+)");

        foreach (const QString& format, se_formats)
        {
            QRegExp exp(format, Qt::CaseInsensitive);
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

    bool Filter::match(const QString& title, QRegExp& exp)
    {
        int pos = 0;
        return ((pos = exp.indexIn(title, pos)) != -1);
    }

    bool Filter::match(Syndication::ItemPtr item)
    {
        bool found_match = false;
        foreach (const QRegExp& exp, word_matches)
        {
            QRegExp tmp = exp;
            tmp.setCaseSensitivity(case_sensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
            tmp.setPatternSyntax(use_regular_expressions ? QRegExp::RegExp : QRegExp::Wildcard);
            if (all_word_matches_must_match)
            {
                if (!match(item->title(), tmp))
                    return false;
                else
                    found_match = true;
            }
            else if (match(item->title(), tmp))
            {
                found_match = true;
                break;
            }
        }

        if (!found_match)
            return false;

        found_match = false;
        foreach (const QRegExp& exp, exclusion_patterns)
        {
            QRegExp tmp = exp;
            tmp.setCaseSensitivity(exclusion_case_sensitive ? Qt::CaseSensitive : Qt::CaseInsensitive);
            tmp.setPatternSyntax(exclusion_reg_exp ? QRegExp::RegExp : QRegExp::Wildcard);
            if (exclusion_all_must_match)
            {
                if (!match(item->title(), tmp))
                {
                    found_match = false;
                    break;
                }
                else
                    found_match = true;
            }
            else if (match(item->title(), tmp))
                return false;
        }

        if (found_match)
            return false;

        if (use_season_and_episode_matching)
        {
            int season = 0;
            int episode = 0;
            if (!getSeasonAndEpisode(item->title(), season, episode))
                return false;

            bool found = false;
            foreach (const Range& r, seasons)
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
            foreach (const Range& r, episodes)
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
                MatchedSeasonAndEpisode se = {season, episode};
                if (se_matches.contains(se))
                    return false;

                se_matches.append(se);
            }
        }

        return true;
    }

    void Filter::addWordMatch(const QRegExp& exp)
    {
        word_matches.append(exp);
    }

    void Filter::removeWordMatch(const QRegExp& exp)
    {
        word_matches.removeAll(exp);
    }

    void Filter::addExclusionPattern(const QRegExp& exp)
    {
        exclusion_patterns.append(exp);
    }

    void Filter::removeExclusionPattern(const QRegExp& exp)
    {
        exclusion_patterns.removeAll(exp);
    }

    bool Filter::stringToRange(const QString& s, Range& r)
    {
        QString tmp = s.trimmed(); // Get rid of whitespace
        if (tmp.contains(QLatin1Char('-')))
        {
            // It's a range
            QStringList parts = s.split(QStringLiteral("-"));
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

    bool Filter::parseNumbersString(const QString& s, QList<Range> & numbers)
    {
        QList<Range> results;
        const QStringList parts = s.split(QStringLiteral(","));
        for (const QString& p : parts)
        {
            Range r = {0, 0};
            if (stringToRange(p, r))
                results.append(r);
            else
                return false;
        }

        numbers.clear();
        numbers = results;
        return true;
    }

    bool Filter::validSeasonOrEpisodeString(const QString& s)
    {
        QList<Range> tmp;
        return Filter::parseNumbersString(s, tmp);
    }

    bool Filter::setSeasons(const QString& s)
    {
        if (parseNumbersString(s, seasons))
        {
            seasons_string = s;
            return true;
        }
        return false;
    }

    bool Filter::setEpisodes(const QString& s)
    {
        if (parseNumbersString(s, episodes))
        {
            episodes_string = s;
            return true;
        }
        return false;
    }

    void Filter::save(bt::BEncoder& enc)
    {
        enc.beginDict();
        enc.write(QByteArrayLiteral("id"), id.toUtf8());
        enc.write(QByteArrayLiteral("name"), name.toUtf8());
        enc.write(QByteArrayLiteral("case_sensitive"), case_sensitive);
        enc.write(QByteArrayLiteral("all_word_matches_must_match"), all_word_matches_must_match);
        enc.write(QByteArrayLiteral("exclusion_case_sensitive"), exclusion_case_sensitive);
        enc.write(QByteArrayLiteral("exclusion_all_must_match"), exclusion_all_must_match);
        enc.write(QByteArrayLiteral("word_matches"));
        enc.beginList();
        foreach (const QRegExp& exp, word_matches)
            enc.write(exp.pattern().toUtf8());
        enc.end();
        enc.write(QByteArrayLiteral("exclusion_patterns"));
        enc.beginList();
        foreach (const QRegExp& exp, exclusion_patterns)
            enc.write(exp.pattern().toUtf8());
        enc.end();
        enc.write(QByteArrayLiteral("use_season_and_episode_matching"), use_season_and_episode_matching);
        enc.write(QByteArrayLiteral("no_duplicate_se_matches"), no_duplicate_se_matches);
        enc.write(QByteArrayLiteral("seasons"), seasons_string.toUtf8());
        enc.write(QByteArrayLiteral("episodes"), episodes_string.toUtf8());
        enc.write(QByteArrayLiteral("download_matching"), download_matching);
        enc.write(QByteArrayLiteral("download_non_matching"), download_non_matching);
        if (!dest_group.isEmpty())
            enc.write(QByteArrayLiteral("group"), dest_group.toUtf8());
        if (!download_location.isEmpty())
            enc.write(QByteArrayLiteral("download_location"), download_location.toUtf8());
        if (!move_on_completion_location.isEmpty())
            enc.write(QByteArrayLiteral("move_on_completion_location"), move_on_completion_location.toUtf8());
        enc.write(QByteArrayLiteral("silently"), silent);
        enc.write(QByteArrayLiteral("use_regular_expressions"), use_regular_expressions);
        enc.write(QByteArrayLiteral("exclusion_reg_exp"), exclusion_reg_exp);
        enc.end();
    }

    bool Filter::load(bt::BDictNode* dict)
    {
        QTextCodec* codec = QTextCodec::codecForName("UTF-8");
        BValueNode* vn = dict->getValue("name");
        if (!vn)
            return false;

        name = vn->data().toString(codec);

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

        vn = dict->getValue("exclusion_case_sensitive");
        if (vn)
            exclusion_case_sensitive = vn->data().toInt() == 1;

        vn = dict->getValue("exclusion_all_must_match");
        if (vn)
            exclusion_all_must_match = vn->data().toInt() == 1;

        BListNode* ln = dict->getList("word_matches");
        if (!ln)
            return false;

        for (Uint32 i = 0; i < ln->getNumChildren(); i++)
        {
            vn = ln->getValue(i);
            if (vn)
                word_matches.append(QRegExp(vn->data().toString(codec), case_sensitive ? Qt::CaseSensitive : Qt::CaseInsensitive));
        }

        ln = dict->getList("exclusion_patterns");
        if (ln)
        {
            for (Uint32 i = 0; i < ln->getNumChildren(); i++)
            {
                vn = ln->getValue(i);
                if (vn)
                    exclusion_patterns.append(QRegExp(vn->data().toString(codec), exclusion_case_sensitive ? Qt::CaseSensitive : Qt::CaseInsensitive));
            }
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

        setSeasons(vn->data().toString(codec));

        vn = dict->getValue("episodes");
        if (!vn)
            return false;

        setEpisodes(vn->data().toString(codec));

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
            setGroup(vn->data().toString(codec));

        vn = dict->getValue("download_location");
        if (vn)
            setDownloadLocation(vn->data().toString(codec));

        vn = dict->getValue("move_on_completion_location");
        if (vn)
            setMoveOnCompletionLocation(vn->data().toString(codec));

        vn = dict->getValue("silently");
        if (!vn)
            return false;

        silent = vn->data().toInt() == 1;

        vn = dict->getValue("use_regular_expressions");
        if (vn)
            use_regular_expressions = vn->data().toInt() == 1;

        vn = dict->getValue("exclusion_reg_exp");
        if (vn)
            exclusion_reg_exp = vn->data().toInt() == 1;

        return true;
    }

    void Filter::startMatching()
    {
        se_matches.clear();
    }
}
