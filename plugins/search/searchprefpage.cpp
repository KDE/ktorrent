/***************************************************************************
 *   Copyright (C) 2005-2007 by Joris Guisson, Ivan Vasic                  *
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
#include <kurl.h>
#include <qtooltip.h>
#include <qfile.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kactivelabel.h>
#include <kpushbutton.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <kio/copyjob.h>
#include <kio/jobuidelegate.h>
#include <klineedit.h>

#include <qlabel.h>
#include <qcheckbox.h>
#include <qradiobutton.h>

#include <util/log.h>
#include <util/constants.h>
#include <util/functions.h>
#include <util/fileops.h>
#include <interfaces/functions.h>
#include "searchprefpage.h"
#include "searchplugin.h"
#include "searchenginelist.h"
#include "searchpluginsettings.h"

using namespace bt;

namespace kt
{
	SearchPrefPage::SearchPrefPage(SearchPlugin* plugin,QWidget* parent)
		: PrefPageInterface(SearchPluginSettings::self(),i18n("Search"), "edit-find",parent), m_plugin(plugin)
	{
		setupUi(this);
		QString foobar = "FOOBAR";
		QString info = i18n("Use your web browser to search for the string %1"
				" (capital letters) on the search engine you want to add. <br> "
				"Then copy the URL in the addressbar after the search is finished, and paste it here.<br><br>Searching for %1"
				" on Google for example, will result in http://www.google.com/search?q=FOOBAR&ie=UTF-8&oe=UTF-8. <br> "
				"If you add this URL here, ktorrent can search using ,Google.",foobar);
		QString info_short = i18n("Use your web browser to search for the string %1 (capital letters) "
				"on the search engine you want to add. Use the resulting URL below.",foobar);
		m_info_label->setText(info_short);
		m_info_label->setToolTip(info);
		
		connect(m_add, SIGNAL(clicked()), this, SLOT(addClicked()));
		connect(m_remove, SIGNAL(clicked()), this, SLOT(removeClicked()));
		connect(m_add_default, SIGNAL(clicked()), this, SLOT(addDefaultClicked()));
		connect(m_remove_all, SIGNAL(clicked()), this, SLOT(removeAllClicked()));
		connect(m_clear_history,SIGNAL(clicked()),this,SLOT(clearHistory()));
		connect(m_update,SIGNAL(clicked()),this,SLOT(updateClicked()));
		
		connect(kcfg_useCustomBrowser, SIGNAL(toggled(bool)), this, SLOT(customToggled( bool )));
		connect(kcfg_openInExternal,SIGNAL(toggled(bool)),this, SLOT(openInExternalToggled(bool)));
		QButtonGroup* bg = new QButtonGroup(this);
		bg->addButton(kcfg_useCustomBrowser);
		bg->addButton(kcfg_useDefaultBrowser);
	}


	SearchPrefPage::~SearchPrefPage()
	{}
	
	void SearchPrefPage::loadSettings()
	{
		updateSearchEngines(m_plugin->getSearchEngineList());
		openInExternalToggled(SearchPluginSettings::openInExternal());
	}

	void SearchPrefPage::loadDefaults()
	{
		loadSettings();
	}
	
	void SearchPrefPage::updateSearchEngines(const SearchEngineList & se)
	{
		m_engines->clear();
		
		for (Uint32 i = 0;i < se.getNumEngines();i++)
			newItem(se.getEngineName(i),se.getSearchURL(i));
	}
 
	void SearchPrefPage::saveSearchEngines()
	{
		QFile fptr(kt::DataDir() + "search_engines");
		if (!fptr.open(QIODevice::WriteOnly))
			return;
		QTextStream out(&fptr);
		out << "# PLEASE DO NOT MODIFY THIS FILE. Use KTorrent configuration dialog for adding new search engines." << ::endl;
		out << "# SEARCH ENGINES list" << ::endl;
     
		QTreeWidgetItemIterator itr(m_engines);
		while (*itr)
		{
			QTreeWidgetItem* item = *itr;
			QString u = item->text(1);
			QString name = item->text(0);
			out << name.replace(" ","%20") << " " << u.replace(" ","%20") <<  endl;
			itr++;
		}
		engineListUpdated();
	}
 
	void SearchPrefPage::addClicked()
	{
		if ( m_engine_url->text().isEmpty() || m_engine_name->text().isEmpty() )
		{
			KMessageBox::error(this, i18n("You must enter the search engine's name and URL"));
		}
		else if ( m_engine_url->text().contains("FOOBAR")  )
		{
			KUrl url = KUrl(m_engine_url->text());
			if ( !url.isValid() ) 
			{ 
				KMessageBox::error(this, i18n("Malformed URL.")); 
				return; 
			}
			
			if (m_engines->findItems(m_engine_name->text(),Qt::MatchExactly, 0).count() > 0) 
			{
				KMessageBox::error(this, i18n("A search engine with the same name already exists. Please use a different name.")); return; 
			}
			

			newItem(m_engine_name->text(),m_engine_url->text());
			m_engine_url->clear();
			m_engine_name->clear();
		}
		else
		{
			KMessageBox::error(this, i18n("Bad URL. You should search for FOOBAR with your Internet browser and copy/paste the exact URL here."));
		}
		saveSearchEngines();
	}
 
	void SearchPrefPage::removeClicked()
	{
		QList<QTreeWidgetItem*> items = m_engines->selectedItems();
		foreach (QTreeWidgetItem* item,items)
		{
			delete item;
		}
		saveSearchEngines();
	}
 
	QTreeWidgetItem* SearchPrefPage::newItem(const QString & name,const KUrl & url)
	{
		// lets not add duplicates
		if (m_engines->findItems(name,Qt::MatchExactly,0).count() > 0)
			return 0;

		QTreeWidgetItem* i = new QTreeWidgetItem(m_engines);
		i->setText(0,name);
		i->setText(1,url.prettyUrl());
		return i;
	}

	void SearchPrefPage::addDefaultClicked()
	{
		const SearchEngineList & se = m_plugin->getSearchEngineList();
		foreach (const SearchEngine & e,se.defaultList())
		{
			newItem(e.name,e.url);
		}

		saveSearchEngines();
	}
 
	void SearchPrefPage::removeAllClicked()
	{
		m_engines->clear();
		saveSearchEngines();
	}
	
	void SearchPrefPage::engineDownloadJobDone(KJob* j)
	{
		if (j->error())
		{
			((KIO::Job*)j)->ui()->showErrorMessage();
			return;
		}
		Out(SYS_SRC|LOG_DEBUG) << "Downloaded search_engines file" << endl;
		QString fn = kt::DataDir() + "search_engines.tmp";
		updateList(fn);
		saveSearchEngines();
		bt::Delete(fn);
	}

	
	void SearchPrefPage::updateClicked()
	{
		QString fn = kt::DataDir() + "search_engines.tmp";
		KIO::Job* j = KIO::copy(KUrl("http://www.ktorrent.org/downloads/search_engines"),KUrl(fn));
		connect(j,SIGNAL(result(KJob*)),this,SLOT(engineDownloadJobDone(KJob*)));	
	}
	
	void SearchPrefPage::updateList(QString& source)
	{
		QFile fptr(source);
     
		if (!fptr.open(QIODevice::ReadOnly))
		{
			Out(SYS_SRC|LOG_DEBUG) << "Failed to open " << source << endl;
			return;
		}
 
		QTextStream in(&fptr);
		
		QMap<QString,KUrl> engines;
		
		while (!in.atEnd())
		{
			QString line = in.readLine();

			if(line.startsWith("#") || line.startsWith(" ") || line.isEmpty() )
				continue;

			QStringList tokens = line.split(" ");
			QString name = tokens[0];
			name = name.replace("%20"," ");
			
			KUrl url = KUrl(tokens[1]);
			for(Uint32 i=2; i<tokens.count(); ++i)
				url.addQueryItem(tokens[i].section("=",0,0), tokens[i].section("=", 1, 1));
			
			engines.insert(name,url);
		}
		
		QMap<QString,KUrl>::iterator i = engines.begin();
		while (i != engines.end())
		{	
			newItem(i.key(),i.value());
			i++;
		}
	}
	
	void SearchPrefPage::customToggled(bool toggled)
	{
		kcfg_customBrowser->setEnabled(toggled);
	}

	void SearchPrefPage::openInExternalToggled(bool on)
	{
		if (on)
		{
			kcfg_useCustomBrowser->setEnabled(true);
			kcfg_customBrowser->setEnabled(SearchPluginSettings::useCustomBrowser());
			kcfg_useDefaultBrowser->setEnabled(true);
		}
		else
		{
			kcfg_useCustomBrowser->setEnabled(false);
			kcfg_customBrowser->setEnabled(false);
			kcfg_useDefaultBrowser->setEnabled(false);
		}
	}

	void SearchPrefPage::clearHistory()
	{
		emit clearSearchHistory();
	}
}

#include "searchprefpage.moc"
