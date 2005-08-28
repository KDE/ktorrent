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

#include <kapplication.h>
#include <khtmlview.h>
#include <qlayout.h>
#include <klineedit.h>
#include <kpushbutton.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kcombobox.h>
#include <kpopupmenu.h>
#include <kparts/partmanager.h>
#include <libutil/log.h>
#include <libtorrent/globals.h>
#include "searchwidget.h"
#include "searchbar.h"
#include "htmlpart.h"
#include "settings.h"



using namespace bt;

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

	right_click_menu = new KPopupMenu(this);
	right_click_menu->insertSeparator();
	back_id = right_click_menu->insertItem(
			KGlobal::iconLoader()->loadIconSet("back",KIcon::Small),
			i18n("Back"),html_part,SLOT(back()));
	right_click_menu->insertItem(
			KGlobal::iconLoader()->loadIconSet("reload",KIcon::Small),
			i18n("Reload"),html_part,SLOT(reload()));

	right_click_menu->setItemEnabled(back_id,false);
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
	
	
	connect(html_part,SIGNAL(backAvailable(bool )),
			this,SLOT(onBackAvailable(bool )));
	connect(html_part,SIGNAL(onURL(const QString& )),
			this,SLOT(onURLHover(const QString& )));
	connect(html_part,SIGNAL(openTorrent(const KURL& )),
			this,SLOT(onOpenTorrent(const KURL& )));
	connect(html_part,SIGNAL(popupMenu(const QString&, const QPoint& )),
			this,SLOT(showPopupMenu(const QString&, const QPoint& )));
	connect(html_part,SIGNAL(searchFinished()),this,SLOT(onFinished()));

	KParts::PartManager* pman = html_part->partManager();
	connect(pman,SIGNAL(partAdded(KParts::Part*)),this,SLOT(onFrameAdded(KParts::Part* )));

	KComboBox* cb = sbar->m_search_engine;

	for (int i = 0;i < NUM_SEARCH_ENGINES;i++)
		cb->insertItem(g_search_engines[i].name);

	cb->setCurrentItem(Settings::searchEngine());

	connect(KApplication::kApplication(),SIGNAL(shutDown()),this,SLOT(onShutDown()));
}


SearchWidget::~SearchWidget()
{
	Settings::setSearchEngine(sbar->m_search_engine->currentItem());
	Settings::writeConfig();
}

void SearchWidget::onBackAvailable(bool available)
{
	sbar->m_back->setEnabled(available);
	right_click_menu->setItemEnabled(back_id,available);
}

void SearchWidget::onFrameAdded(KParts::Part* p)
{
	KHTMLPart* frame = dynamic_cast<KHTMLPart*>(p);
	if (frame)
	{
		connect(frame,SIGNAL(popupMenu(const QString&, const QPoint& )),
				this,SLOT(showPopupMenu(const QString&, const QPoint& )));
	}
}

void SearchWidget::copy()
{
	if (!html_part)
		return;
	html_part->copy();
}

void SearchWidget::search(const QString & text, const int engine)
{
	if (!html_part)
		return;
	KURL url;
	searchUrl(&url, text, engine);

	statusBarMsg(i18n("Searching for %1 ...").arg(text));
	//html_part->openURL(url);
	html_part->openURLRequest(url,KParts::URLArgs());
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

void SearchWidget::onFinished()
{
	statusBarMsg(i18n("Search finished"));
}

void SearchWidget::onOpenTorrent(const KURL & url)
{
	openTorrent(url);
}

void SearchWidget::showPopupMenu(const QString & url,const QPoint & p)
{
	right_click_menu->popup(p);
}

KPopupMenu* SearchWidget::rightClickMenu()
{
	return right_click_menu;
}

void SearchWidget::onShutDown()
{
	delete html_part;
	html_part = 0;
}

#include "searchwidget.moc"
