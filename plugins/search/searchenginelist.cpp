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
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QDir>
#include <klocale.h>
#include <kio/copyjob.h>
#include <util/log.h>
#include <util/error.h>
#include <util/fileops.h>
#include <kstandarddirs.h>
#include "searchenginelist.h"
#include "opensearchdownloadjob.h"

using namespace bt;

namespace kt
{ 
	QString DataDir();

	SearchEngineList::SearchEngineList(const QString & data_dir) : data_dir(data_dir)
	{
		default_opensearch_urls << KUrl("http://www.torrentz.com") << KUrl("http://isohunt.com");
		default_urls << KUrl("http://www.ktorrents.com")
				<< KUrl("http://www.bittorrent.com")
				<< KUrl("http://www.mininova.org")
				<< KUrl("http://thepiratebay.org")
				<< KUrl("http://www.bitenova.org")
				<< KUrl("http://btjunkie.org");
	}


	SearchEngineList::~SearchEngineList()
	{
		qDeleteAll(engines);
	}
	
	
	void SearchEngineList::loadEngines()
	{
		if (!bt::Exists(data_dir))
		{
			if (bt::Exists(kt::DataDir() + "search_engines"))
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
				Out(SYS_SRC|LOG_DEBUG) << "Setting up default engines" << endl;
				addDefaults();
			}
		}
		else
		{
			QStringList subdirs = QDir(data_dir).entryList(QDir::Dirs);
			foreach (const QString & sd,subdirs)
			{
				if (sd == ".." || sd == ".")
					continue;
				
				// Load only if there is an opensearch.xml file and not a removed file
				if (bt::Exists(data_dir + sd + "/opensearch.xml") && !bt::Exists(data_dir + sd + "/removed"))
				{
					Out(SYS_SRC|LOG_DEBUG) << "Loading " << sd << endl;
					SearchEngine* se = new SearchEngine(data_dir + sd + "/");
					if (!se->load(data_dir + sd + "/opensearch.xml"))
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
		QFile fptr(kt::DataDir() + "search_engines");
		if (!fptr.open(QIODevice::ReadOnly))
		{
			addDefaults();
			return;
		}
		
		QTextStream in(&fptr);
		
		while (!in.atEnd())
		{
			QString line = in.readLine();
		
			if(line.startsWith("#") || line.startsWith(" ") || line.isEmpty() ) continue;
		
			QStringList tokens = line.split(" ");
			QString name = tokens[0];
			name = name.replace("%20"," ");
			KUrl url = KUrl(tokens[1]);
		
			for (Uint32 i=2; i < (Uint32)tokens.count(); ++i)
				url.addQueryItem(tokens[i].section("=",0,0), tokens[i].section("=", 1, 1));
		
			try
			{
				QString dir = data_dir + name;
				if (!dir.endsWith("/"))
					dir += "/";
				
				bt::MakeDir(dir);
				addEngine(dir,url.prettyUrl().replace("FOOBAR","{searchTerms}"));
			}
			catch (bt::Error & err)
			{
				Out(SYS_SRC|LOG_NOTICE) << err.toString() << endl;
			}
		}
	}
		
	KUrl SearchEngineList::search(bt::Uint32 engine,const QString & terms)
	{
		KUrl u;
		if (engine < (Uint32)engines.count())
			u = engines[engine]->search(terms);
		
		Out(SYS_SRC|LOG_NOTICE) << "Searching " << u.prettyUrl() << endl;
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
		if (osdj->error())
			bt::Delete(osdj->directory(),true);
		
		SearchEngine* se = new SearchEngine(osdj->directory());
		if (!se->load(osdj->directory() + "opensearch.xml"))
		{
			delete se;
			bt::Delete(osdj->directory(),true);
		}
		else
			engines.append(se);
		
		insertRow(engines.count() - 1);
	}
	
	void SearchEngineList::addEngine(OpenSearchDownloadJob* j)
	{
		openSearchDownloadJobFinished(j);
	}
	
	void SearchEngineList::addEngine(const QString & dir,const QString & url)
	{
		QFile fptr(dir + "opensearch.xml");
		if (!fptr.open(QIODevice::WriteOnly))
			throw bt::Error(i18n("Cannot open %1 : %2",dir + "opensearch.xml",fptr.errorString()));
		
		KUrl kurl(url);
		QTextStream out(&fptr);
		QString xml_template = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
				"<OpenSearchDescription xmlns=\"http://a9.com/-/spec/opensearch/1.1/\">\n"
				"<ShortName>%1</ShortName>\n"
				"<Url type=\"text/html\" template=\"%2\" />\n"
				"<Image>%3/favicon.ico</Image>\n"
				"</OpenSearchDescription>\n";
		
		QString base = kurl.protocol() + "://" + kurl.host();
		if (kurl.port() > 0)
			base += QString(":%1").arg(kurl.port());
		
		QString tmp = url;
		tmp = tmp.replace("&","&amp;");
		out << xml_template.arg(kurl.host()).arg(tmp).arg(base) << endl;
		
		SearchEngine* se = new SearchEngine(dir);
		
		if (!se->load(dir + "opensearch.xml"))
		{
			delete se;
			throw bt::Error(i18n("Failed to parse %1",dir + "opensearch.xml"));
		}
		
		engines.append(se);
		insertRow(engines.count() - 1);
	}
	
	void SearchEngineList::removeEngines(const QModelIndexList & sel)
	{
		QList<SearchEngine*> to_remove;
		foreach (const QModelIndex & idx,sel)
		{
			if (idx.isValid() && idx.row() >= 0 && idx.row() < engines.count())
				to_remove.append(engines.at(idx.row()));
		}
		
		foreach (SearchEngine* se,to_remove)
		{
			bt::Touch(se->engineDir() + "removed");
			engines.removeAll(se);
			delete se;
		}
		
		reset();
	}
	
	void SearchEngineList::removeAllEngines()
	{
		removeRows(0,engines.count(),QModelIndex());
		engines.clear();
		reset();
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
	
		foreach (const KUrl & u,default_opensearch_urls)
		{
			Out(SYS_SRC|LOG_DEBUG) << "Setting up default engine " << u.prettyUrl() << endl;
			QString dir = data_dir + u.host() + "/"; 
			if (!bt::Exists(dir))
			{
				OpenSearchDownloadJob* j = new OpenSearchDownloadJob(u,dir);
				connect(j,SIGNAL(result(KJob*)),this,SLOT(openSearchDownloadJobFinished(KJob*)));
				j->start();
			}
			else
			{
				loadEngine(dir,dir,true);
			}
		}
			
		// also add the engines which don't have an opensearch description
		loadDefault(true);
		reset();
	}
	
	void SearchEngineList::loadEngine(const QString& global_dir, const QString& user_dir,bool load_removed)
	{
		if (!bt::Exists(user_dir))
		{
			// create directory to store icons
			bt::MakeDir(user_dir);
		}
		
		if (bt::Exists(user_dir + "removed"))
		{
			// if the removed file is there don't load, if we are not allowed
			if (!load_removed)
				return;
			else
				bt::Delete(user_dir + "removed");
		}
		
		if (!alreadyLoaded(user_dir))
		{
			SearchEngine* se = new SearchEngine(user_dir);
			if (!se->load(global_dir + "opensearch.xml"))
				delete se;
			else
				engines.append(se);
		}
	}

	
	void SearchEngineList::loadDefault(bool removed_to)
	{
		QStringList dir_list = KGlobal::dirs()->findDirs("data", "ktorrent/opensearch");
		foreach (const QString & dir,dir_list)
		{
			QStringList subdirs = QDir(dir).entryList(QDir::Dirs);
			foreach (const QString & sd,subdirs)
			{
				if (sd == ".." || sd == ".")
					continue;
				
				loadEngine(dir + sd + "/",data_dir + sd + "/",removed_to);
			}
		}
	}

	bool SearchEngineList::alreadyLoaded(const QString& user_dir)
	{
		foreach (const SearchEngine* se,engines)
		{
			if (se->engineDir() == user_dir)
				return true;
		}
		
		return false;
	}
	
	int SearchEngineList::rowCount(const QModelIndex &parent) const
	{
		if (!parent.isValid())
			return engines.count();
		else
			return 0;
	}
	
	QVariant SearchEngineList::data(const QModelIndex &index, int role) const
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
			return i18n("Url: <b>%1</b>",se->engineUrl());
		}
		
		return QVariant();
	}
	
	bool SearchEngineList::insertRows(int row,int count,const QModelIndex & parent)
	{
		Q_UNUSED(parent);
		beginInsertRows(QModelIndex(),row,row + count - 1);
		endInsertRows();
		return true;
	}
	
	bool SearchEngineList::removeRows(int row,int count,const QModelIndex & parent)
	{
		Q_UNUSED(parent);
		beginRemoveRows(QModelIndex(),row,row + count - 1);
		for (int i = 0;i < count;i++)
		{
			SearchEngine* se = engines.takeAt(row);
			bt::Touch(se->engineDir() + "removed");
			delete se;
		}
		endRemoveRows();
		return true;
	}
}
