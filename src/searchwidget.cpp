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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <khtmlview.h>
#include <qlayout.h>
#include <klineedit.h>
#include <kpushbutton.h>
#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>
#include "searchwidget.h"
#include "searchbar.h"
#include "htmlpart.h"


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
			KGlobal::iconLoader()->loadIconSet("clear_left",KIcon::Small));
	sbar->m_back->setIconSet(
			KGlobal::iconLoader()->loadIconSet("back",KIcon::Small));
	sbar->m_reload->setIconSet(
			KGlobal::iconLoader()->loadIconSet("reload",KIcon::Small));
	
	
	connect(html_part,SIGNAL(backAvailable(bool )),sbar->m_back,SLOT(setEnabled(bool )));
	connect(html_part,SIGNAL(onURL(const QString& )),this,SLOT(onURLHover(const QString& )));
	connect(html_part->view(),SIGNAL(finishedLayout()),this,SLOT(onFinishedLayout()));
	connect(html_part,SIGNAL(openTorrent(const KURL& )),this,SLOT(onOpenTorrent(const KURL& )));
}


SearchWidget::~SearchWidget()
{}

void SearchWidget::copy()
{
	html_part->copy();
}

void SearchWidget::search(const QString & text)
{
	KURL url = "http://search.bittorrent.com/search.jsp";
	url.addQueryItem("query",text);
	url.addQueryItem("Submit2","Search");

	statusBarMsg(i18n("Searching for %1 ...").arg(text));
	html_part->openURL(url);
}

void SearchWidget::searchPressed()
{
	search(sbar->m_search_text->text());
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
