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

#include "searchwidget.h"

#include <QLabel>
#include <QClipboard>
#include <QProgressBar>
#include <QVBoxLayout>
#include <KIconLoader>
#include <KComboBox>
#include <KLocale>
#include <QApplication>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <KMenu>
#include <KStandardAction>
#include <kio/job.h>
#include <KMessageBox>
#include <KFileDialog>
#include <KActionCollection>

#include <util/log.h>
#include <magnet/magnetlink.h>
#include <torrent/globals.h>
#include <interfaces/guiinterface.h>
#include <interfaces/coreinterface.h>
#include <interfaces/functions.h>
#include "searchplugin.h"
#include "searchenginelist.h"
#include "webview.h"
#include "searchactivity.h"


using namespace bt;

namespace kt
{
	
	SearchWidget::SearchWidget(SearchPlugin* sp) : webview(0),sp(sp),prog(0),torrent_download(0)
	{
		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->setSpacing(0);
		layout->setMargin(0);
		webview = new WebView(this);
		
		KActionCollection* ac = sp->getSearchActivity()->part()->actionCollection();
		sbar = new KToolBar(this);
		sbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
		sbar->addAction(webview->pageAction(QWebPage::Back));
		sbar->addAction(webview->pageAction(QWebPage::Forward));
		sbar->addAction(webview->pageAction(QWebPage::Reload));
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
		layout->addWidget(webview);

		right_click_menu = new KMenu(this);
		open_url_action = right_click_menu->addAction(KIcon("tab-new"),i18n("Open in New Tab"),this,SLOT(openNewTab()));
		open_url_action->setEnabled(false);
		right_click_menu->addSeparator();
		right_click_menu->addAction(webview->pageAction(QWebPage::Back));
		right_click_menu->addAction(webview->pageAction(QWebPage::Reload));
		right_click_menu->addSeparator();
		right_click_menu->addAction(webview->pageAction(QWebPage::Copy));
		copy_url_action = right_click_menu->addAction(KIcon("edit-copy"),i18n("Copy URL"),this,SLOT(copyUrl()));
		search_text->setClearButtonShown(true);
		
		connect(webview,SIGNAL(loadStarted()),this,SLOT(loadStarted()));
		connect(webview,SIGNAL(loadFinished(bool)),this,SLOT(loadFinished(bool)));
		connect(webview,SIGNAL(loadProgress(int)),this,SLOT(loadProgress(int)));
		connect(webview->page(),SIGNAL(unsupportedContent(QNetworkReply*)),
				this,SLOT(unsupportedContent(QNetworkReply*)));
		connect(webview,SIGNAL(linkMiddleOrCtrlClicked(KUrl)),this,SIGNAL(openNewTab(KUrl)));
		connect(webview,SIGNAL(iconChanged()),this,SLOT(iconChanged()));
		connect(webview,SIGNAL(titleChanged(QString)),this,SLOT(titleChanged(QString)));
	}
	
	
	SearchWidget::~SearchWidget()
	{
		if (prog)
		{
			sp->getGUI()->getStatusBar()->removeProgressBar(prog);
			prog = 0;
		}
	}
	
	void SearchWidget::iconChanged()
	{
		changeIcon(this,webview->icon());
	}
	
	void SearchWidget::titleChanged(const QString& text)
	{
		changeTitle(this,text);
	}

	void SearchWidget::copyUrl()
	{
		QClipboard* cb = QApplication::clipboard();
		cb->setText(url_to_open.prettyUrl());
	}
	
	KUrl SearchWidget::getCurrentUrl() const
	{
		return webview->url();
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
		if (url.protocol() == "home")
			webview->openUrl(KUrl("about:ktorrent"));
		else
			webview->openUrl(url);
	
		search_text->setText(sb_text);
		search_engine->setCurrentIndex(engine);
	}
	
	void SearchWidget::search(const QString & text,int engine)
	{
		if (search_text->text() != text)
			search_text->setText(text);
		
		if (search_engine->currentIndex() != engine)
			search_engine->setCurrentIndex(engine);
	
		KUrl url = sp->getSearchEngineList()->search(engine,text);
		webview->openUrl(url);
	}
	
	/*
	void SearchWidget::onSearchRequested(const QString & text)
	{
		search(text,search_engine->currentIndex());
	}
	*/
	
	void SearchWidget::setSearchBarEngine(int engine)
	{
		search_engine->setCurrentIndex(engine);
	}
	/*
	void SearchWidget::showPopupMenu(const QString & url,const QPoint & p)
	{
		open_url_action->setEnabled(!url.isEmpty());
		copy_url_action->setEnabled(!url.isEmpty());
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
		
		right_click_menu->popup(p);
	}
	
	*/
	
	KMenu* SearchWidget::rightClickMenu()
	{
		return right_click_menu;
	}
	
	void SearchWidget::loadProgress(int perc)
	{
		if (!prog)
			prog = sp->getGUI()->getStatusBar()->createProgressBar();
		
		if (prog)
			prog->setValue(perc);
	}
	
	void SearchWidget::loadStarted()
	{
		if (!prog)
		{
			prog = sp->getGUI()->getStatusBar()->createProgressBar();
			if (prog)
				prog->setValue(0);
		}
	}
	
	void SearchWidget::loadFinished(bool ok)
	{
		Q_UNUSED(ok);
		if (prog)
		{
			sp->getGUI()->getStatusBar()->removeProgressBar(prog);
			prog = 0;
		}
	}

	void SearchWidget::unsupportedContent(QNetworkReply* r)
	{
		if (r->url().scheme() == "magnet")
		{
			sp->getCore()->load(bt::MagnetLink(r->url().toString()),QString());
		}
		else if (r->header(QNetworkRequest::ContentTypeHeader).toString() == "application/x-bittorrent" || 
				 r->url().path().endsWith(".torrent"))
		{
			torrent_download = r;
			if (!r->isFinished())
				connect(r,SIGNAL(finished()),this,SLOT(downloadRequestFinished()));
			else
				downloadRequestFinished();
		}
		else
		{
			KMessageBox::error(this,QString("unsupportedContent %1").arg(r->url().toString()));
			r->abort();
		}
	}
	
	void SearchWidget::saveReply(QNetworkReply* reply)
	{
		QString fn = KFileDialog::getSaveFileName(KUrl("kfiledialog:///openTorrent"),kt::TorrentFileFilter(false),this);
		if (!fn.isNull())
		{
			QFile fptr(fn);
			if (!fptr.open(QIODevice::WriteOnly))
			{
				KMessageBox::error(this,i18n("Cannot open <b>%1</b>: %2",fn,fptr.errorString()));
			}
			else
			{
				fptr.write(reply->readAll());
			}
		}
	}

	
	void SearchWidget::downloadRequestFinished()
	{
		if (torrent_download->error() != QNetworkReply::NoError)
			return;
		
		int ret = KMessageBox::questionYesNoCancel(0,
			i18n("Do you want to download or save the torrent?"),
			i18n("Download Torrent"),
			KGuiItem(i18n("Download"),"ktorrent"),
			KStandardGuiItem::save());
		
		if (ret == KMessageBox::Yes)
			sp->getCore()->load(torrent_download->readAll(),torrent_download->url(),QString(),QString());
		else if (ret == KMessageBox::No)
			saveReply(torrent_download);
		
		torrent_download = 0;
	}

	
	void SearchWidget::search()
	{
		search(search_text->text(),search_engine->currentIndex());
	}

	void SearchWidget::openNewTab()
	{
		openNewTab(url_to_open);
	}
	
	void SearchWidget::home()
	{
		webview->home();
	}
	
	bool SearchWidget::backAvailable() const
	{
		return webview->pageAction(QWebPage::Back)->isEnabled();
	}
}

#include "searchwidget.moc"
