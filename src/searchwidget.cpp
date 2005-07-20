/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
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
 *   51 Franklin Steet, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/

#include <qapplication.h>
#include <khtmlview.h>
#include <qlayout.h>
#include <klineedit.h>
#include <kpushbutton.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kcombobox.h>
#include "searchwidget.h"
#include "searchbar.h"
#include "htmlpart.h"
#include "settings.h"

struct SearchEngine
{
	QString name,url;
	int id;
};

const int NUM_SEARCH_ENGINES = 7;

static SearchEngine g_search_engines[] =
{
	{"bittorrent.com","http://search.bittorrent.com/search.jsp",0},
	{"isohunt.com" ,"http://isohunt.com/torrents.php",1},
	{"thepiratebay.org","http://thepiratebay.org/search.php",2},
	{"bitoogle.com","http://search.bitoogle.com/search.php",3},
	{"bytenova.org","http://www.bytenova.org/search.php",4},
	{"torrentspy.com","http://torrentspy.com/search.asp",5},
	{"mininova.org","http://www.mininova.org/search.php",6}
};


SearchWidget::SearchWidget(QWidget* parent,const char* name)
: QWidget(parent,name),html_part(0)
{
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setAutoAdd(true);
	sbar = new SearchBar(this);
	html_part = new HTMLPart(this);

	sbar->m_back->setEnabled(false);
	connect(sbar->m_search_button,SIGNAL(clicked()),this,SLOT(searchPressed()));
	connect(sbar->m_clear_button,SIGNAL(clicked()),this,SLOT(clearPressed()));
	connect(sbar->m_search_text,SIGNAL(returnPressed()),this,SLOT(searchPressed()));
	connect(sbar->m_back,SIGNAL(clicked()),html_part,SLOT(back()));
	connect(sbar->m_reload,SIGNAL(clicked()),html_part,SLOT(reload()));

	sbar->m_clear_button->setIconSet(
			KGlobal::iconLoader()->loadIconSet(QApplication::reverseLayout() ? "clear_left" : "locationbar_erase",KIcon::Small));
	sbar->m_back->setIconSet(
			KGlobal::iconLoader()->loadIconSet("back",KIcon::Small));
	sbar->m_reload->setIconSet(
			KGlobal::iconLoader()->loadIconSet("reload",KIcon::Small));
	
	
	connect(html_part,SIGNAL(backAvailable(bool )),sbar->m_back,SLOT(setEnabled(bool )));
	connect(html_part,SIGNAL(onURL(const QString& )),this,SLOT(onURLHover(const QString& )));
	connect(html_part->view(),SIGNAL(finishedLayout()),this,SLOT(onFinishedLayout()));
	connect(html_part,SIGNAL(openTorrent(const KURL& )),this,SLOT(onOpenTorrent(const KURL& )));

	KComboBox* cb = sbar->m_search_engine;

	for (int i = 0;i < NUM_SEARCH_ENGINES;i++)
		cb->insertItem(g_search_engines[i].name);

	cb->setCurrentItem(Settings::searchEngine());
}


SearchWidget::~SearchWidget()
{
	Settings::setSearchEngine(sbar->m_search_engine->currentItem());
	Settings::writeConfig();
}

void SearchWidget::copy()
{
	html_part->copy();
}

void SearchWidget::search(const QString & text, const int engine)
{
	KURL url;
	searchUrl(&url, text, engine);

	statusBarMsg(i18n("Searching for %1 ...").arg(text));
	html_part->openURL(url);
}



void SearchWidget::searchUrl(KURL* url, const QString& text, const int engine)
{
	if (engine < NUM_SEARCH_ENGINES)
		*url = g_search_engines[engine].url;
	else
		*url = g_search_engines[0].url;
	
	switch(engine)
	{
		case 1: // ISOHUNT
			url->addQueryItem("ihq",text);
			url->addQueryItem("op","and");
			break;
		case 2: //PirateBay.org
			url->addQueryItem("q", text);
			break;
		case 3: //BITOOGLE
			url->addQueryItem("q", text);
			url->addQueryItem("st","t");
			break;
		case 4: //ByteNova
			url->addQueryItem("search", text);
			break;
		case 5: //TorrentSpy
			url->addQueryItem("query",text);
			break;
		case 6: //Mininova
			url->addQueryItem("search", text);
			break;
		case 0:
		default:
			url->addQueryItem("query",text);
			url->addQueryItem("Submit2","Search");
	}
}

void SearchWidget::searchPressed()
{
	search(sbar->m_search_text->text(),sbar->m_search_engine->currentItem());
}

void SearchWidget::clearPressed()
{
	sbar->m_search_text->clear();
}

void SearchWidget::onURLHover(const QString & url)
{
	statusBarMsg(url);
}

void SearchWidget::onFinishedLayout()
{
	statusBarMsg(i18n("Finished"));
}

void SearchWidget::onOpenTorrent(const KURL & url)
{
	openTorrent(url);
}

#include "searchwidget.moc"
