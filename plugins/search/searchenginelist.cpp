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

#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QStringList>
#include <QTextStream>
#include <QUrlQuery>

#include <KIO/CopyJob>
#include <KLocalizedString>

#include <util/log.h>
#include <util/error.h>
#include <util/fileops.h>
#include <interfaces/functions.h>
#include "searchenginelist.h"
#include "opensearchdownloadjob.h"

using namespace bt;

namespace kt
{

    SearchEngineList::SearchEngineList(ProxyHelper *proxy, const QString& data_dir) : m_proxy(proxy), data_dir(data_dir)
    {
        // default_opensearch_urls << QUrl(QStringLiteral("https://torrentproject.com"));
        default_urls << QUrl(QStringLiteral("http://btdig.com"))
                     << QUrl(QStringLiteral("http://btdb.in"))
                     << QUrl(QStringLiteral("https://torrentproject.se"))
                     << QUrl(QStringLiteral("http://duckduckgo.com"));
    }


    SearchEngineList::~SearchEngineList()
    {
        qDeleteAll(engines);
    }


    void SearchEngineList::loadEngines()
    {
        if (!bt::Exists(data_dir))
        {
            if (bt::Exists(kt::DataDir() + QStringLiteral("search_engines")))
            {
                try
                {
                    if (!bt::Exists(data_dir))
                        bt::MakeDir(data_dir);
                }
                catch (...)
                {
                    return;
                }

                convertSearchEnginesFile();
            }
            else
            {
                Out(SYS_SRC | LOG_DEBUG) << "Setting up default engines" << endl;
                addDefaults();
            }
        }
        else
        {
            QStringList subdirs = QDir(data_dir).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
            for (const QString& sd : qAsConst(subdirs))
            {

                // Load only if there is an opensearch.xml file and not a removed file
                if (bt::Exists(data_dir + sd + QStringLiteral("/opensearch.xml")) && !bt::Exists(data_dir + sd + QStringLiteral("/removed")))
                {
                    Out(SYS_SRC | LOG_DEBUG) << "Loading " << sd << endl;
                    SearchEngine* se = new SearchEngine(data_dir + sd + QLatin1Char('/'));
                    if (!se->load(data_dir + sd + QStringLiteral("/opensearch.xml")))
                        delete se;
                    else
                        engines.append(se);
                }
            }

            // check if new engines have been added
            loadDefault(false);
        }
    }

    void SearchEngineList::convertSearchEnginesFile()
    {
        QFile fptr(kt::DataDir() + QStringLiteral("search_engines"));
        if (!fptr.open(QIODevice::ReadOnly))
        {
            addDefaults();
            return;
        }

        QTextStream in(&fptr);

        while (!in.atEnd())
        {
            QString line = in.readLine();

            if (line.startsWith(QLatin1Char('#')) || line.startsWith(QLatin1Char(' ')) || line.isEmpty()) continue;

            QStringList tokens = line.split(QLatin1Char(' '));
            QString name = tokens[0];
            name = name.replace(QLatin1String("%20"), QLatin1String(" "));
            QUrlQuery url = QUrlQuery(QUrl(tokens[1]));

            for (Uint32 i = 2; i < (Uint32)tokens.count(); ++i)
                url.addQueryItem(tokens[i].section(QLatin1Char('='), 0, 0), tokens[i].section(QLatin1Char('='), 1, 1));

            try
            {
                QString dir = data_dir + name;
                if (!dir.endsWith(QLatin1Char('/')))
                    dir += QLatin1Char('/');

                bt::MakeDir(dir);
                addEngine(dir, url.toString().replace(QLatin1String("FOOBAR"), QLatin1String("{searchTerms}")));
            }
            catch (bt::Error& err)
            {
                Out(SYS_SRC | LOG_NOTICE) << err.toString() << endl;
            }
        }
    }

    QUrl SearchEngineList::search(bt::Uint32 engine, const QString& terms)
    {
        QUrl u;
        if (engine < (Uint32)engines.count())
            u = engines[engine]->search(terms);

        Out(SYS_SRC | LOG_NOTICE) << "Searching " << u.toDisplayString() << endl;
        return u;
    }

    QString SearchEngineList::getEngineName(bt::Uint32 engine) const
    {
        if (engine >= (Uint32)engines.count())
            return QString::null;
        else
            return engines[engine]->engineName();
    }

    void SearchEngineList::openSearchDownloadJobFinished(KJob* j)
    {
        OpenSearchDownloadJob* osdj = (OpenSearchDownloadJob*)j;
        if (!osdj->error())
        {
            SearchEngine* se = new SearchEngine(osdj->directory());
            if (!se->load(osdj->directory() + QStringLiteral("opensearch.xml")))
            {
                delete se;
                bt::Delete(osdj->directory(), true);
            }
            else {
                engines.append(se);
                insertRow(engines.count() - 1);
            }
        }
        else {
            bt::Delete(osdj->directory(), true);
        }
    }

    void SearchEngineList::addEngine(OpenSearchDownloadJob* j)
    {
        openSearchDownloadJobFinished(j);
    }

    void SearchEngineList::addEngine(const QString& dir, const QString& url)
    {
        QFile fptr(dir + QStringLiteral("opensearch.xml"));
        if (!fptr.open(QIODevice::WriteOnly))
            throw bt::Error(i18n("Cannot open %1: %2", dir + QStringLiteral("opensearch.xml"), fptr.errorString()));

        QUrl kurl(url);
        QTextStream out(&fptr);
        QString xml_template = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                               "<OpenSearchDescription xmlns=\"http://a9.com/-/spec/opensearch/1.1/\">\n"
                               "<ShortName>%1</ShortName>\n"
                               "<Url type=\"text/html\" template=\"%2\" />\n"
                               "<Image>%3/favicon.ico</Image>\n"
                               "</OpenSearchDescription>\n");

        QString base = kurl.scheme() + QStringLiteral("://") + kurl.host();
        if (kurl.port() > 0)
            base += QString::fromLatin1(":%1").arg(kurl.port());

        QString tmp = url;
        tmp = tmp.replace(QLatin1Char('&'), QLatin1String("&amp;"));
        out << xml_template.arg(kurl.host()).arg(tmp).arg(base) << endl;

        SearchEngine* se = new SearchEngine(dir);

        if (!se->load(dir + QStringLiteral("opensearch.xml")))
        {
            delete se;
            throw bt::Error(i18n("Failed to parse %1", dir + QStringLiteral("opensearch.xml")));
        }

        engines.append(se);
        insertRow(engines.count() - 1);
    }

    void SearchEngineList::removeEngines(const QModelIndexList& sel)
    {
        QList<SearchEngine*> to_remove;
        for (const QModelIndex& idx : sel)
        {
            if (idx.isValid() && idx.row() >= 0 && idx.row() < engines.count())
                to_remove.append(engines.at(idx.row()));
        }

        beginResetModel();
        for (SearchEngine* se : qAsConst(to_remove))
        {
            bt::Touch(se->engineDir() + QStringLiteral("removed"));
            engines.removeAll(se);
            delete se;
        }
        endResetModel();
    }

    void SearchEngineList::removeAllEngines()
    {
        beginResetModel();
        removeRows(0, engines.count(), QModelIndex());
        engines.clear();
        endResetModel();
    }

    void SearchEngineList::addDefaults()
    {
        // data dir does not exist yet so create it and add the default list
        try
        {
            if (!bt::Exists(data_dir))
                bt::MakeDir(data_dir);
        }
        catch (...)
        {
            return;
        }

        beginResetModel();
        for (const QUrl &u : qAsConst(default_opensearch_urls))
        {
            Out(SYS_SRC | LOG_DEBUG) << "Setting up default engine " << u.toDisplayString() << endl;
            QString dir = data_dir + u.host() + QLatin1Char('/');
            if (!bt::Exists(dir))
            {
                OpenSearchDownloadJob* j = new OpenSearchDownloadJob(u, dir, m_proxy);
                connect(j, &OpenSearchDownloadJob::result, this, &SearchEngineList::openSearchDownloadJobFinished);
                j->start();
            }
            else
            {
                loadEngine(dir, dir, true);
            }
        }

        // also add the engines which don't have an opensearch description
        loadDefault(true);
        endResetModel();
    }

    void SearchEngineList::loadEngine(const QString& global_dir, const QString& user_dir, bool load_removed)
    {
        if (!bt::Exists(user_dir))
        {
            // create directory to store icons
            bt::MakeDir(user_dir);
        }

        if (bt::Exists(user_dir + QStringLiteral("removed")))
        {
            // if the removed file is there don't load, if we are not allowed
            if (!load_removed)
                return;
            else
                bt::Delete(user_dir + QStringLiteral("removed"));
        }

        if (!alreadyLoaded(user_dir))
        {
            SearchEngine* se = new SearchEngine(user_dir);
            if (!se->load(global_dir + QStringLiteral("opensearch.xml")))
                delete se;
            else
                engines.append(se);
        }
    }


    void SearchEngineList::loadDefault(bool removed_to)
    {
        QStringList dir_list = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("ktorrent/opensearch"), QStandardPaths::LocateDirectory);
        if (dir_list.isEmpty())
            dir_list = QStandardPaths::locateAll(QStandardPaths::DataLocation, QStringLiteral("ktorrent/opensearch"), QStandardPaths::LocateDirectory);
        if (dir_list.isEmpty())
            dir_list = QStandardPaths::locateAll(QStandardPaths::AppDataLocation, QStringLiteral("ktorrent/opensearch"), QStandardPaths::LocateDirectory);

        for (const QString& dir : qAsConst(dir_list))
        {
            const QStringList subdirs = QDir(dir).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
            for (const QString& sd : subdirs)
            {
                loadEngine(QDir::cleanPath(dir) + QLatin1Char('/') + sd + QLatin1Char('/'), data_dir + sd + QLatin1Char('/'), removed_to);
            }
        }
    }

    bool SearchEngineList::alreadyLoaded(const QString& user_dir)
    {
        for (const SearchEngine* se : qAsConst(engines))
        {
            if (se->engineDir() == user_dir)
                return true;
        }

        return false;
    }

    int SearchEngineList::rowCount(const QModelIndex& parent) const
    {
        if (!parent.isValid())
            return engines.count();
        else
            return 0;
    }

    QVariant SearchEngineList::data(const QModelIndex& index, int role) const
    {
        if (!index.isValid())
            return QVariant();

        SearchEngine* se = engines.at(index.row());
        if (!se)
            return QVariant();

        if (role == Qt::DisplayRole)
        {
            return se->engineName();
        }
        else if (role == Qt::DecorationRole)
        {
            return se->engineIcon();
        }
        else if (role == Qt::ToolTipRole)
        {
            return i18n("URL: <b>%1</b>", se->engineUrl());
        }

        return QVariant();
    }

    bool SearchEngineList::insertRows(int row, int count, const QModelIndex& parent)
    {
        Q_UNUSED(parent);
        beginInsertRows(QModelIndex(), row, row + count - 1);
        endInsertRows();
        return true;
    }

    bool SearchEngineList::removeRows(int row, int count, const QModelIndex& parent)
    {
        Q_UNUSED(parent);
        beginRemoveRows(QModelIndex(), row, row + count - 1);
        for (int i = 0; i < count; i++)
        {
            SearchEngine* se = engines.takeAt(row);
            bt::Touch(se->engineDir() + QLatin1String("removed"));
            delete se;
        }
        endRemoveRows();
        return true;
    }
}
