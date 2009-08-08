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
#include <kactioncollection.h>
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
	
	SearchWidget::SearchWidget(SearchPlugin* sp) : html_part(0),sp(sp)
	{
		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->setSpacing(0);
		layout->setMargin(0);
		html_part = new HTMLPart(this);
		
		KActionCollection* ac = sp->actionCollection();
		sbar = new KToolBar(this);
		sbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
		sbar->addAction(ac->action("search_tab_back"));
		sbar->addAction(ac->action("search_tab_reload"));
		sbar->addAction(ac->action("search_home"));
		search_text = new KLineEdit(sbar);
		sbar->addWidget(search_text);
		sbar->addAction(ac->action("search_tab_search"));
		sbar->addWidget(new QLabel(i18n(" Engine:")));
		search_engine = new KComboBox(sbar);
		search_engine->setModel(sp->getSearchEngineList());
		sbar->addWidget(search_engine);
		
		connect(search_text,SIGNAL(returnPressed()),this,SLOT(search()));;

		layout->addWidget(sbar);
		layout->addWidget(html_part->view());
		html_part->show();
		html_part->view()->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);

		right_click_menu = new KMenu(this);
		open_url_action = right_click_menu->addAction(KIcon("tab-new"),i18n("Open in New Tab"),this,SLOT(openNewTab()));
		open_url_action->setEnabled(false);
		right_click_menu->addSeparator();
		right_click_menu->addAction(ac->action("search_tab_back"));
		right_click_menu->addAction(ac->action("search_tab_reload"));
		right_click_menu->addSeparator();
		right_click_menu->addAction(ac->action("search_tab_copy"));
		

		search_text->setClearButtonShown(true);

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
	
	void SearchWidget::onBackAvailable(bool available)
	{
		enableBack(available);
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
		return search_text->text();
	}
	
	int SearchWidget::getSearchBarEngine() const
	{
		return search_engine->currentIndex();
	}
	
	void SearchWidget::restore(const KUrl & url,const QString & text,const QString & sb_text,int engine)
	{
		if (html_part)
		{
			if (url.protocol() == "home")
				home();
			else
			{
				html_part->openUrl(url);
				html_part->addToHistory(url);
			}
		}
	
		search_text->setText(sb_text);
		search_engine->setCurrentIndex(engine);
	}
	
	void SearchWidget::search(const QString & text,int engine)
	{
		if (!html_part)
			return;
		
		if (search_text->text() != text)
			search_text->setText(text);
		
		if (search_engine->currentIndex() != engine)
			search_engine->setCurrentIndex(engine);
	
		KUrl url = sp->getSearchEngineList()->search(engine,text);
	
		statusBarMsg(i18n("Searching for %1...",text));
		//html_part->openURL(url);
 		html_part->openUrlRequest(url,KParts::OpenUrlArguments(),KParts::BrowserArguments());
		at_home = false;
	}
	
	void SearchWidget::setSearchBarEngine(int engine)
	{
		search_engine->setCurrentIndex(engine);
	}
	
	void SearchWidget::onUrlHover(const QString & url)
	{
		statusBarMsg(url);
	}
	
	void SearchWidget::onFinished()
	{
		changeTitle(this,html_part->title());
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
	
	void SearchWidget::showPopupMenu(const QString & url,const QPoint & p)
	{
		open_url_action->setEnabled(!url.isEmpty());
		right_click_menu->popup(p);
		if (!url.isEmpty())
		{
			if (!url.startsWith("/"))
				url_to_open = KUrl(url);
			else 
			{
				KUrl u = html_part->baseURL();
				QString base = u.scheme() + "://" + u.authority();
				url_to_open = KUrl(base);
				url_to_open.setPath(url);
			}
		}
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
		sp->getCore()->load(url,QString());
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
	
	void SearchWidget::find()
	{
		html_part->findText();
	}
	
	
	void SearchWidget::search()
	{
		search(search_text->text(),search_engine->currentIndex());
	}
	
	void SearchWidget::back()
	{
		html_part->back();
	}
	
	void SearchWidget::reload()
	{
		if (atHome())
			home();
		else
			html_part->reload();
	}
	
	void SearchWidget::openNewTab()
	{
		openNewTab(url_to_open);
	}
	
	void SearchWidget::home() 
	{
		html_part->begin();
		html_part->write("<html><head></head><body></body></html>");
		html_part->end();
		changeTitle(this,i18n("Home"));
		at_home = true;
	}

	
	bool SearchWidget::backAvailable() const
	{
		return html_part->backAvailable();
	}
}

#include "searchwidget.moc"
