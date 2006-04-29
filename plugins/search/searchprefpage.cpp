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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#include <kurl.h>
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
#include <util/constants.h>
#include "searchprefpage.h"

using namespace bt;

namespace kt
{
	SearchPrefPageWidget::SearchPrefPageWidget(QWidget *parent) : SEPreferences(parent)
	{
		QString info = i18n("Use your web browser to search for the string %1"
				" (capital letters) on the search engine you want to add. Then copy the URL in the addressbar after the search is finished, and paste it here.<br>Searching for %2"
				" on Google for example, will result in http://www.google.com/search?q=FOOBAR&ie=UTF-8&oe=UTF-8. If you add this URL here, ktorrent can search using Google.").arg("FOOBAR").arg("FOOBAR");
		m_infoLabel->setText(info);
		loadSearchEngines();
		connect(btnAdd, SIGNAL(clicked()), this, SLOT(addClicked()));
		connect(btnRemove, SIGNAL(clicked()), this, SLOT(removeClicked()));
		connect(btn_add_default, SIGNAL(clicked()), this, SLOT(addDefaultClicked()));
		connect(btnRemoveAll, SIGNAL(clicked()), this, SLOT(removeAllClicked()));
	}
 
	bool SearchPrefPageWidget::apply()
	{
		saveSearchEngines();
		return true;
	}
 
	void SearchPrefPageWidget::loadSearchEngines()
	{
		m_items.clear();
		m_engines->clear();
		
		QFile fptr(KGlobal::dirs()->saveLocation("data","ktorrent") + "search_engines");
     
		if (!fptr.open(IO_ReadOnly))
			return;
 
		QTextStream in(&fptr);
		int id = 0;

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
		
			QListViewItem* se = new QListViewItem(m_engines, name, url.url());
			m_items.append(se);
			m_engines->insertItem(se);

			++id;
		}
	}
 
	void SearchPrefPageWidget::saveSearchEngines()
	{
		QFile fptr(KGlobal::dirs()->saveLocation("data","ktorrent") + "search_engines");
		if (!fptr.open(IO_WriteOnly))
			return;
		QTextStream out(&fptr);
		out << "# PLEASE DO NOT MODIFY THIS FILE. Use KTorrent configuration dialog for adding new search engines." << ::endl;
		out << "# SEARCH ENGINES list" << ::endl;
     
		for(Uint32 i=0; i<m_items.count(); ++i)
		{
			QListViewItem* item = m_items.at(i);
			QString u = item->text(1);
			QMap<QString,QString> args = KURL(u).queryItems();

			// replace spaces by %20
			QString name = item->text(0);
			name = name.replace(" ","%20");
	
			out << name << " " << u.section("?",0,0) << " ";
			
			for (QMap<QString,QString>::iterator j = args.begin();j != args.end();j++)
				out << j.key() << "=" << j.data() << " ";
			out << endl;
		}
	}
 
	void SearchPrefPageWidget::addClicked()
	{
		if ( m_engine_url->text().isEmpty() || m_engine_name->text().isEmpty() )
			KMessageBox::error(this, i18n("You must enter the search engine's name and URL"), i18n("Error"));
		else if ( m_engine_url->text().contains("FOOBAR")  )
		{
			KURL url = KURL::fromPathOrURL(m_engine_url->text());
			if ( !url.isValid() ) { KMessageBox::error(this, i18n("Malformed URL."), i18n("Error")); return; }
			if (m_engines->findItem(m_engine_name->text(), 0)) { KMessageBox::error(this, i18n("A search engine with the same name already exists. Please use a different name.")); return; }
			QListViewItem* se = new QListViewItem(m_engines, m_engine_name->text(), m_engine_url->text());
			m_engines->insertItem(se);
			m_items.append(se);
			m_engine_url->setText("");
			m_engine_name->setText("");
		}
		else
			KMessageBox::error(this, i18n("Bad URL. You should search for FOOBAR with your Internet browser and copy/paste the exact URL here."));
	}
 
	void SearchPrefPageWidget::removeClicked()
	{
		QListView trt;
		if ( m_engines->selectedItem() == 0 ) return;
     
		QListViewItem* item = m_engines->selectedItem();
		m_engines->takeItem(item);
		m_items.remove(item);
	}
 
	void SearchPrefPageWidget::addDefaultClicked()
	{
		QListViewItem* se = new QListViewItem(m_engines, "KTorrents", "http://www.ktorrents.com/search.php?lg=0&sourceid=ktorrent&q=FOOBAR&f=0");
		m_items.append(se);
		m_engines->insertItem(se);
		
		se = new QListViewItem(m_engines, "bittorrent.com", "http://search.bittorrent.com/search.jsp?query=FOOBAR");
		m_items.append(se);
		m_engines->insertItem(se);
     
		se = new QListViewItem(m_engines, "isohunt.com", "http://isohunt.com/torrents.php?ihq=FOOBAR&op=and");
		m_items.append(se);
		m_engines->insertItem(se);
     
		se = new QListViewItem(m_engines, "mininova.org", "http://www.mininova.org/search.php?search=FOOBAR");
		m_items.append(se);
		m_engines->insertItem(se);
     
		se = new QListViewItem(m_engines, "thepiratebay.org", "http://thepiratebay.org/search.php?q=FOOBAR");
		m_items.append(se);
		m_engines->insertItem(se);
     
		se = new QListViewItem(m_engines, "bitoogle.com", "http://search.bitoogle.com/search.php?q=FOOBAR&st=t");
		m_items.append(se);
		m_engines->insertItem(se);
     
		se = new QListViewItem(m_engines, "bytenova.org", "http://www.bitenova.org/search.php?search=FOOBAR&start=0&start=0&ie=utf-8&oe=utf-8");
		m_items.append(se);
		m_engines->insertItem(se);
     
		se = new QListViewItem(m_engines, "torrentspy.com", "http://torrentspy.com/search.asp?query=FOOBAR");
		m_items.append(se);
		m_engines->insertItem(se);

		se = new QListViewItem(m_engines, "torrentz.com", "http://www.torrentz.com/search_FOOBAR");
		m_items.append(se);
		m_engines->insertItem(se);
	}
 
	void SearchPrefPageWidget::removeAllClicked()
	{
		m_engines->clear();
		m_items.clear();
	}
	

	SearchPrefPage::SearchPrefPage()
	: PrefPageInterface(i18n("a noun", "Search"), i18n("Search Engine Options"),
						KGlobal::iconLoader()->loadIcon("viewmag",KIcon::NoGroup))
	{
		widget = 0;
	}


	SearchPrefPage::~SearchPrefPage()
	{}


	bool SearchPrefPage::apply()
	{
		return widget->apply();
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
		widget->loadSearchEngines();
	}

}

#include "searchprefpage.moc"
