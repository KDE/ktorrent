/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson, Ivan Vasic                       *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
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
#include <klistview.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <kio/netaccess.h>
#include <klineedit.h>

#include <qlabel.h>
#include <qcheckbox.h>
#include <qradiobutton.h>

#include <util/constants.h>
#include "searchprefpage.h"
#include "searchplugin.h"
#include "searchenginelist.h"
#include "searchpluginsettings.h"

using namespace bt;

namespace kt
{
	SearchPrefPageWidget::SearchPrefPageWidget(QWidget *parent) : SEPreferences(parent)
	{
		QString info = i18n("Use your web browser to search for the string %1"
				" (capital letters) on the search engine you want to add. <br> "
				"Then copy the URL in the addressbar after the search is finished, and paste it here.<br><br>Searching for %1"
				" on Google for example, will result in http://www.google.com/search?q=FOOBAR&ie=UTF-8&oe=UTF-8. <br> "
				"If you add this URL here, ktorrent can search using Google.").arg("FOOBAR").arg("FOOBAR");
		QString info_short = i18n("Use your web browser to search for the string %1 (capital letters) "
				"on the search engine you want to add. Use the resulting URL below.").arg("FOOBAR");
		m_infoLabel->setText(info_short);
		QToolTip::add(m_infoLabel,info);
		QToolTip::add(m_engine_name,info);
		
		connect(btnAdd, SIGNAL(clicked()), this, SLOT(addClicked()));
		connect(btnRemove, SIGNAL(clicked()), this, SLOT(removeClicked()));
		connect(btn_add_default, SIGNAL(clicked()), this, SLOT(addDefaultClicked()));
		connect(btnRemoveAll, SIGNAL(clicked()), this, SLOT(removeAllClicked()));
		
		connect(useCustomBrowser, SIGNAL(toggled(bool)), this, SLOT(customToggled( bool )));
		
		useCustomBrowser->setChecked(SearchPluginSettings::useCustomBrowser());
		useDefaultBrowser->setChecked(SearchPluginSettings::useDefaultBrowser());
		customBrowser->setText(SearchPluginSettings::customBrowser());
		
		customBrowser->setEnabled(useCustomBrowser->isChecked());
		openExternal->setChecked(SearchPluginSettings::openInExternal());
	}
	
	void SearchPrefPageWidget::updateSearchEngines(const SearchEngineList & se)
	{
		m_engines->clear();
		
		for (Uint32 i = 0;i < se.getNumEngines();i++)
		{
			new QListViewItem(m_engines,se.getEngineName(i),se.getSearchURL(i).prettyURL());
		}
	}
 
	bool SearchPrefPageWidget::apply()
	{
		saveSearchEngines();
		
		SearchPluginSettings::setUseCustomBrowser(useCustomBrowser->isChecked());
		SearchPluginSettings::setUseDefaultBrowser(useDefaultBrowser->isChecked());
		SearchPluginSettings::setCustomBrowser(customBrowser->text());
		SearchPluginSettings::setOpenInExternal(openExternal->isChecked());
		SearchPluginSettings::writeConfig();
		return true;
	}
 
	void SearchPrefPageWidget::saveSearchEngines()
	{
		QFile fptr(KGlobal::dirs()->saveLocation("data","ktorrent") + "search_engines");
		if (!fptr.open(IO_WriteOnly))
			return;
		QTextStream out(&fptr);
		out << "# PLEASE DO NOT MODIFY THIS FILE. Use KTorrent configuration dialog for adding new search engines." << ::endl;
		out << "# SEARCH ENGINES list" << ::endl;
     
		QListViewItemIterator itr(m_engines);
		while (itr.current())
		{
			QListViewItem* item = itr.current();
			QString u = item->text(1);
			QString name = item->text(0);
			out << name.replace(" ","%20") << " " << u.replace(" ","%20") <<  endl;
			itr++;
		}
	}
 
	void SearchPrefPageWidget::addClicked()
	{
		if ( m_engine_url->text().isEmpty() || m_engine_name->text().isEmpty() )
		{
			KMessageBox::error(this, i18n("You must enter the search engine's name and URL"));
		}
		else if ( m_engine_url->text().contains("FOOBAR")  )
		{
			KURL url = KURL::fromPathOrURL(m_engine_url->text());
			if ( !url.isValid() ) 
			{ 
				KMessageBox::error(this, i18n("Malformed URL.")); 
				return; 
			}
			
			if (m_engines->findItem(m_engine_name->text(), 0)) 
			{
				KMessageBox::error(this, i18n("A search engine with the same name already exists. Please use a different name.")); return; 
			}
			
			new QListViewItem(m_engines, m_engine_name->text(), m_engine_url->text());
			m_engine_url->setText("");
			m_engine_name->setText("");
		}
		else
		{
			KMessageBox::error(this, i18n("Bad URL. You should search for FOOBAR with your Internet browser and copy/paste the exact URL here."));
		}
	}
 
	void SearchPrefPageWidget::removeClicked()
	{
		if ( m_engines->selectedItem() == 0 ) 
			return;
 
		QListViewItem* item = m_engines->selectedItem();
		m_engines->takeItem(item);
		delete item;
	}
 
	void SearchPrefPageWidget::addDefaultClicked()
	{
		QListViewItem* se = new QListViewItem(m_engines, "KTorrents", "http://www.ktorrents.com/search.php?lg=0&sourceid=ktorrent&q=FOOBAR&f=0");
		
		se = new QListViewItem(m_engines, "bittorrent.com", "http://search.bittorrent.com/search.jsp?query=FOOBAR");
     
		se = new QListViewItem(m_engines, "isohunt.com", "http://isohunt.com/torrents.php?ihq=FOOBAR&op=and");
     
		se = new QListViewItem(m_engines, "mininova.org", "http://www.mininova.org/search.php?search=FOOBAR");
     
		se = new QListViewItem(m_engines, "thepiratebay.org", "http://thepiratebay.org/search.php?q=FOOBAR");
     
		se = new QListViewItem(m_engines, "bitoogle.com", "http://bitoogle.com/search.php?q=FOOBAR");
     
		se = new QListViewItem(m_engines, "bytenova.org", "http://www.bitenova.org/search.php?search=FOOBAR&start=0&start=0&ie=utf-8&oe=utf-8");
     
		se = new QListViewItem(m_engines, "torrentspy.com", "http://torrentspy.com/search.asp?query=FOOBAR");

		se = new QListViewItem(m_engines, "torrentz.com", "http://www.torrentz.com/search_FOOBAR");
	}
 
	void SearchPrefPageWidget::removeAllClicked()
	{
		m_engines->clear();
	}
	
	void SearchPrefPageWidget::btnUpdate_clicked()
	{
		QString fn = KGlobal::dirs()->saveLocation("data","ktorrent") + "search_engines.tmp";
		KURL source("http://www.ktorrent.org/downloads/search_engines");
		
		if (KIO::NetAccess::download(source,fn,NULL))
		{
			//list successfully downloaded, remove temporary file
			updateList(fn);
			saveSearchEngines();
			KIO::NetAccess::removeTempFile(fn);
		}
	}
	
	void SearchPrefPageWidget::updateList(QString& source)
	{
		QFile fptr(source);
     
		if (!fptr.open(IO_ReadOnly))
			return;
 
		QTextStream in(&fptr);
		
		QMap<QString,KURL> engines;
		
		while (!in.atEnd())
		{
			QString line = in.readLine();

			if(line.startsWith("#") || line.startsWith(" ") || line.isEmpty() )
				continue;

			QStringList tokens = QStringList::split(" ", line);
			QString name = tokens[0];
			name = name.replace("%20"," ");
			
			KURL url = KURL::fromPathOrURL(tokens[1]);
			for(Uint32 i=2; i<tokens.count(); ++i)
				url.addQueryItem(tokens[i].section("=",0,0), tokens[i].section("=", 1, 1));
			
			engines.insert(name,url);
		}
		
		QMap<QString,KURL>::iterator i = engines.begin();
		while (i != engines.end())
		{	
			QListViewItem* item = m_engines->findItem(i.key(),0);
			// if we have found the item, replace it if not make a new one
			if (item)
				item->setText(1, i.data().prettyURL());
			else
				new QListViewItem(m_engines,i.key(),i.data().prettyURL());
			
			i++;
		}
	}
	
	////////////////////////////////////////////////////////////////////////////////////////////
	

	SearchPrefPage::SearchPrefPage(SearchPlugin* plugin)
		: PrefPageInterface(i18n("a noun", "Search"), i18n("Search Engine Options"),
							KGlobal::iconLoader()->loadIcon("viewmag",KIcon::NoGroup)), m_plugin(plugin)
	{
		widget = 0;
	}


	SearchPrefPage::~SearchPrefPage()
	{}


	bool SearchPrefPage::apply()
	{
		bool ret = widget->apply();
		if(ret)
			m_plugin->preferencesUpdated();
		
		return ret;
	}

	void SearchPrefPage::createWidget(QWidget* parent)
	{
		widget = new SearchPrefPageWidget(parent);
	}

	void SearchPrefPage::deleteWidget()
	{
		delete widget;
	}

	void SearchPrefPage::updateData()
	{
		widget->updateSearchEngines(m_plugin->getSearchEngineList());
		
	}
	
	void SearchPrefPageWidget::customToggled(bool toggled)
	{
		customBrowser->setEnabled(toggled);
	}
}

#include "searchprefpage.moc"
