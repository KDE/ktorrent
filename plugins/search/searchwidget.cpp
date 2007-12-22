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
#include <QLabel>
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
#include <kmenu.h>
#include <kstandardaction.h>
#include <kparts/partmanager.h>
#include <kio/job.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <QProgressBar>
#include <util/log.h>
#include <torrent/globals.h>
#include <interfaces/guiinterface.h>
#include <interfaces/coreinterface.h>
#include "searchwidget.h"
#include "htmlpart.h"
#include "searchplugin.h"
#include "searchenginelist.h"



using namespace bt;

namespace kt
{
	SearchBar::SearchBar(HTMLPart* html_part,SearchWidget* parent) : KToolBar(parent)
	{
		m_back = KStandardAction::back(html_part,SLOT(back()),this);
		m_back->setEnabled(false);
		addAction(m_back);
		m_reload = KStandardAction::redisplay(html_part,SLOT(reload()),this);
		m_reload->setText(i18n("Reload"));
		addAction(m_reload);
		m_search_text = new KLineEdit(this);
		addWidget(m_search_text);
		m_search = KStandardAction::find(parent,SLOT(searchPressed()),this);
		addAction(m_search);
		addWidget(new QLabel(i18n(" Engine:")));
		m_search_engine = new KComboBox(this);
		addWidget(m_search_engine);
		
		connect(m_search_text,SIGNAL(returnPressed()),parent,SLOT(searchPressed()));
		
		//m_back->setIcon(KStandardGuiItem::back(KStandardGuiItem::UseRTL).icon());
		//m_reload->setIcon(KIcon("view-refresh"));
		//m_back->setGuiItem(KStandardGuiItem::back());
	}

	SearchBar::~SearchBar()
	{}
	
	SearchWidget::SearchWidget(SearchPlugin* sp) : html_part(0),sp(sp)
	{
		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->setSpacing(0);
		layout->setMargin(0);
		html_part = new HTMLPart(this);
		sbar = new SearchBar(html_part,this);

		layout->addWidget(sbar);
		layout->addWidget(html_part->view());
		html_part->show();
		html_part->view()->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

		right_click_menu = new KMenu(this);
		back_action = right_click_menu->addAction(KIcon("go-previous"),i18n("Back"),html_part,SLOT(back()));
		right_click_menu->addAction(KIcon("view-refresh"),i18n("Reload"),html_part,SLOT(reload()));
		back_action->setEnabled(false);

		sbar->m_search_text->setClearButtonShown(true);

		connect(html_part,SIGNAL(backAvailable(bool )),
				this,SLOT(onBackAvailable(bool )));
		connect(html_part,SIGNAL(onURL(const QString& )),
				this,SLOT(onUrlHover(const QString& )));
		connect(html_part,SIGNAL(openTorrent(const KUrl& )),
				this,SLOT(onOpenTorrent(const KUrl& )));
		connect(html_part,SIGNAL(popupMenu(const QString&, const QPoint& )),
				this,SLOT(showPopupMenu(const QString&, const QPoint& )));
		connect(html_part,SIGNAL(searchFinished()),this,SLOT(onFinished()));
		connect(html_part,SIGNAL(saveTorrent(const KUrl& )),
				this,SLOT(onSaveTorrent(const KUrl& )));
	
		KParts::PartManager* pman = html_part->partManager();
		connect(pman,SIGNAL(partAdded(KParts::Part*)),this,SLOT(onFrameAdded(KParts::Part* )));
		
		connect(html_part->browserExtension(),SIGNAL(loadingProgress(int)),this,SLOT(loadingProgress(int)));
		prog = 0;
	}
	
	
	SearchWidget::~SearchWidget()
	{
		if (prog)
		{
			sp->getGUI()->getStatusBar()->removeProgressBar(prog);
			prog = 0;
		}
	}
	
	void SearchWidget::updateSearchEngines(const SearchEngineList & sl)
	{
		int ci = sbar->m_search_engine->currentIndex(); 
		sbar->m_search_engine->clear();
		for (Uint32 i = 0;i < sl.getNumEngines();i++)
		{
			sbar->m_search_engine->addItem(sl.getEngineName(i));
		}
		sbar->m_search_engine->setCurrentIndex(ci);
	}
	
	void SearchWidget::onBackAvailable(bool available)
	{
		sbar->m_back->setEnabled(available);
		back_action->setEnabled(available);
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
	
	KUrl SearchWidget::getCurrentUrl() const
	{
		if (!html_part)
			return KUrl();
		return html_part->toplevelURL();
	}
	
	QString SearchWidget::getSearchBarText() const
	{
		return sbar->m_search_text->text();
	}
	
	int SearchWidget::getSearchBarEngine() const
	{
		return sbar->m_search_engine->currentIndex();
	}
	
	void SearchWidget::restore(const KUrl & url,const QString & text,const QString & sb_text,int engine)
	{
		if (html_part)
			html_part->openUrl(url);
		search_text = text;
		sbar->m_search_text->setText(sb_text);
		sbar->m_search_engine->setCurrentIndex(engine);
	}
	
	void SearchWidget::search(const QString & text,int engine)
	{
		if (!html_part)
			return;
		
		search_text = text;
		
		if (sbar->m_search_text->text() != text)
			sbar->m_search_text->setText(text);
		
		if (sbar->m_search_engine->currentIndex() != engine)
			sbar->m_search_engine->setCurrentIndex(engine);
	
		const SearchEngineList & sl = sp->getSearchEngineList();
		
		if (engine < 0 || (Uint32)engine >= sl.getNumEngines())
			engine = sbar->m_search_engine->currentIndex();
		
		QString s_url = sl.getSearchURL(engine).prettyUrl();
		s_url.replace("FOOBAR", QUrl::toPercentEncoding(text), Qt::CaseSensitive);
		KUrl url = KUrl(s_url);
	
		statusBarMsg(i18n("Searching for %1...",text));
		//html_part->openURL(url);
 		html_part->openUrlRequest(url,KParts::OpenUrlArguments(),KParts::BrowserArguments());
	}	
	
	void SearchWidget::searchPressed()
	{
		search(sbar->m_search_text->text(),sbar->m_search_engine->currentIndex());
	}
	
	void SearchWidget::onUrlHover(const QString & url)
	{
		statusBarMsg(url);
	}
	
	void SearchWidget::onFinished()
	{
	}
	
	void SearchWidget::onOpenTorrent(const KUrl & url)
	{
		openTorrent(url);
	}
	
	void SearchWidget::onSaveTorrent(const KUrl & url)
	{
		QString fn = KFileDialog::getSaveFileName(KUrl("kfiledialog:///openTorrent"),"*.torrent | " + i18n("torrent files"),this);
		if (!fn.isNull())
		{
			KUrl save_url = KUrl(fn);
			// start a copy job
			KIO::file_copy(url,save_url);
		}
	}
	
	void SearchWidget::showPopupMenu(const QString & /*url*/,const QPoint & p)
	{
		right_click_menu->popup(p);
	}
	
	KMenu* SearchWidget::rightClickMenu()
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
		sp->getGUI()->getStatusBar()->message(url);
	}
	
	void SearchWidget::openTorrent(const KUrl & url)
	{
		Out(SYS_GEN|LOG_DEBUG) << "SearchWidget::openTorrent " << url.prettyUrl() << endl;
		sp->getCore()->load(url);
	}
	
	void SearchWidget::loadingProgress(int perc)
	{
		if (perc < 100 && !prog)
		{
			prog = sp->getGUI()->getStatusBar()->createProgressBar();
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
				sp->getGUI()->getStatusBar()->removeProgressBar(prog);
				prog = 0;
			}
			statusBarMsg(i18n("Search finished"));
		}
	}
}

#include "searchwidget.moc"
