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

#include <kapplication.h>
#include <khtmlview.h>
#include <qlayout.h>
#include <qfile.h> 
#include <qtextstream.h> 
#include <qstring.h> 
#include <qstringlist.h> 
#include <klineedit.h>
#include <kpushbutton.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h> 
#include <kiconloader.h>
#include <kcombobox.h>
#include <kpopupmenu.h>
#include <kparts/partmanager.h>
#include <kio/netaccess.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <util/log.h>
#include <torrent/globals.h>
#include <interfaces/guiinterface.h>
#include <interfaces/coreinterface.h>
#include "searchwidget.h"
#include "searchbar.h"
#include "htmlpart.h"
#include "searchpluginsettings.h"
#include "searchplugin.h"



using namespace bt;

namespace kt
{
	
	
	SearchWidget::SearchWidget(SearchPlugin* sp,QWidget* parent,const char* name)
	: QWidget(parent,name),html_part(0),sp(sp)
	{
		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->setAutoAdd(true);
		sbar = new SearchBar(this);
		html_part = new HTMLPart(this);
	
		right_click_menu = new KPopupMenu(this);
		right_click_menu->insertSeparator();
		back_id = right_click_menu->insertItem(
                        KGlobal::iconLoader()->loadIconSet(QApplication::reverseLayout() 
                        ? "forward" : "back",KIcon::Small),
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
				KGlobal::iconLoader()->loadIconSet(QApplication::reverseLayout() 
                        ? "clear_left" : "locationbar_erase",KIcon::Small));
		sbar->m_back->setIconSet(
                        KGlobal::iconLoader()->loadIconSet(QApplication::reverseLayout() 
                        ? "forward" : "back", KIcon::Small));
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
		connect(html_part,SIGNAL(saveTorrent(const KURL& )),
				this,SLOT(onSaveTorrent(const KURL& )));
	
		KParts::PartManager* pman = html_part->partManager();
		connect(pman,SIGNAL(partAdded(KParts::Part*)),this,SLOT(onFrameAdded(KParts::Part* )));
	
		loadSearchEngines();
	}
	
	
	SearchWidget::~SearchWidget()
	{
		SearchPluginSettings::setSearchEngine(sbar->m_search_engine->currentItem());
		SearchPluginSettings::writeConfig();
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
	
	void SearchWidget::search(const QString & text,int engine)
	{
		if (!html_part || m_search_engines.count() == 0)
			return;
	
		if (engine < 0 || (Uint32)engine >= m_search_engines.count())
			engine = sbar->m_search_engine->currentItem();
		
		QString s_url = m_search_engines[engine].url.url();
		s_url.replace("FOOBAR", KURL::encode_string(text), true);
		KURL url = KURL::fromPathOrURL(s_url);
	
		statusBarMsg(i18n("Searching for %1...").arg(text));
		//html_part->openURL(url);
		html_part->openURLRequest(url,KParts::URLArgs());
	}
	
	void SearchWidget::loadSearchEngines() 
	{
		m_search_engines.clear();
		QFile fptr(KGlobal::dirs()->saveLocation("data","ktorrent") + "search_engines");
		
		if(!fptr.exists())
			makeDefaultSearchEngines();
		
		if (!fptr.open(IO_ReadOnly))
			return;
		
		QTextStream in(&fptr);
		
		int id = 0;
		
		while (!in.atEnd())
		{
			QString line = in.readLine();
		
			if(line.startsWith("#") || line.startsWith(" ") || line.isEmpty() ) continue;
		
			QStringList tokens = QStringList::split(" ", line);
		
			SearchEngine se;
			se.name = tokens[0];
			se.name = se.name.replace("%20"," ");
			se.url = KURL::fromPathOrURL(tokens[1]);
			se.id = id;
		
			for(Uint32 i=2; i<tokens.count(); ++i)
				se.url.addQueryItem(tokens[i].section("=",0,0), tokens[i].section("=", 1, 1));
		
			m_search_engines.append(se);
		
			++id;
		}
	
		sbar->m_search_engine->clear();
		for(Uint32 i=0; i<m_search_engines.count(); ++i)
			sbar->m_search_engine->insertItem(m_search_engines[i].name);
		sbar->m_search_engine->setCurrentItem(SearchPluginSettings::searchEngine());
	}
	
	void SearchWidget::makeDefaultSearchEngines()
	{
		QFile fptr(KGlobal::dirs()->saveLocation("data","ktorrent") + "search_engines");
		if (!fptr.open(IO_WriteOnly))
			return;
		QTextStream out(&fptr);
		out << "# PLEASE DO NOT MODIFY THIS FILE. Use KTorrent configuration dialog for adding new search engines." << ::endl;
		out << "# SEARCH ENGINES list" << ::endl;
		out << "KTorrents http://www.ktorrents.com/search.php?lg=0&sourceid=ktorrent&q=FOOBAR&f=0" << ::endl;
		out << "bittorrent.com http://www.bittorrent.com/search_result.myt?search=FOOBAR" << ::endl; 
		out << "isohunt.com http://isohunt.com/torrents.php?ihq=FOOBAR&op=and" << ::endl; 
		out << "mininova.org http://www.mininova.org/search.php?search=FOOBAR" << ::endl; 
		out << "thepiratebay.org http://thepiratebay.org/search.php?q=FOOBAR" << ::endl; 
		out << "bitoogle.com http://search.bitoogle.com/search.php?q=FOOBAR&st=t" << ::endl; 
		out << "bytenova.org http://www.bitenova.org/search.php?search=FOOBAR&start=0&start=0&ie=utf-8&oe=utf-8" << ::endl; 
		out << "torrentspy.com http://torrentspy.com/search.asp?query=FOOBAR" << ::endl; 
		out << "torrentz.com http://www.torrentz.com/search_FOOBAR" << ::endl; 
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
	
	void SearchWidget::onSaveTorrent(const KURL & url)
	{
		KFileDialog fdlg(QString::null,"*.torrent | " + i18n("torrent files"),this,0,true);
		fdlg.setSelection(url.fileName());
		fdlg.setOperationMode(KFileDialog::Saving);
		if (fdlg.exec() == QDialog::Accepted)
		{
			KURL save_url = fdlg.selectedURL();
			if (!KIO::NetAccess::copy(url,save_url,this))
				KMessageBox::error(this,KIO::NetAccess::lastErrorString());
		}
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

	void SearchWidget::statusBarMsg(const QString & url)
	{
		sp->getGUI()->changeStatusbar(url);
	}
	
	void SearchWidget::openTorrent(const KURL & url)
	{
		sp->getCore()->load(url);
	}
}

#include "searchwidget.moc"
