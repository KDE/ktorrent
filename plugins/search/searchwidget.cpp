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
#include <kio/job.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kprogress.h>
#include <util/log.h>
#include <torrent/globals.h>
#include <interfaces/guiinterface.h>
#include <interfaces/coreinterface.h>
#include "searchwidget.h"
#include "searchbar.h"
#include "htmlpart.h"
#include "searchplugin.h"
#include "searchenginelist.h"



using namespace bt;

namespace kt
{
	
	
	SearchWidget::SearchWidget(SearchPlugin* sp) : html_part(0),sp(sp)
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
		
		connect(html_part->browserExtension(),SIGNAL(loadingProgress(int)),this,SLOT(loadingProgress(int)));
		prog = 0;
	}
	
	
	SearchWidget::~SearchWidget()
	{
		if (prog)
		{
			sp->getGUI()->removeProgressBarFromStatusBar(prog);
			prog = 0;
		}
	}
	
	void SearchWidget::updateSearchEngines(const SearchEngineList & sl)
	{
		int ci = sbar->m_search_engine->currentItem(); 
		sbar->m_search_engine->clear();
		for (Uint32 i = 0;i < sl.getNumEngines();i++)
		{
			sbar->m_search_engine->insertItem(sl.getEngineName(i));
		}
		sbar->m_search_engine->setCurrentItem(ci);
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
		if (!html_part)
			return;
		
		if (sbar->m_search_text->text() != text)
			sbar->m_search_text->setText(text);
		
		if (sbar->m_search_engine->currentItem() != engine)
			sbar->m_search_engine->setCurrentItem(engine);
	
		const SearchEngineList & sl = sp->getSearchEngineList();
		
		if (engine < 0 || (Uint32)engine >= sl.getNumEngines())
			engine = sbar->m_search_engine->currentItem();
		
		QString s_url = sl.getSearchURL(engine).prettyURL();
		s_url.replace("FOOBAR", KURL::encode_string(text), true);
		KURL url = KURL::fromPathOrURL(s_url);
	
		statusBarMsg(i18n("Searching for %1...").arg(text));
		//html_part->openURL(url);
 		html_part->openURLRequest(url,KParts::URLArgs());
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
			// start a copy job
			KIO::Job* j = KIO::file_copy(url,save_url,-1,true);
			// let it deal with the errors
			j->setAutoErrorHandlingEnabled(true,0);
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
	
	void SearchWidget::loadingProgress(int perc)
	{
		if (perc < 100 && !prog)
		{
			prog = sp->getGUI()->addProgressBarToStatusBar();
			if (prog)
				prog->setValue(perc);
		}
		else if (prog && perc < 100)
		{
			prog->setValue(perc);
		}
		else if (perc == 100) 
		{
			if (prog)
			{
				sp->getGUI()->removeProgressBarFromStatusBar(prog);
				prog = 0;
			}
			statusBarMsg(i18n("Search finished"));
		}
	}
}

#include "searchwidget.moc"
