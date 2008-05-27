/***************************************************************************
 *   Copyright (C) 2005-2007 by Joris Guisson                              *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include <QAction>
#include <qfile.h>
#include <qtextstream.h>
#include <qapplication.h>
#include <qcheckbox.h>
#include <KLineEdit>
#include <kicon.h>
#include <kglobal.h>
#include <kguiitem.h>
#include <kpushbutton.h>
#include <kiconloader.h>
#include <kcombobox.h>
#include <kcompletion.h>
#include <kmainwindow.h>
#include <qlabel.h>
#include <klocale.h>
#include <kstandardguiitem.h>
#include <kactioncollection.h>
#include <ktoolbarlabelaction.h>
#include <interfaces/functions.h>
#include <util/fileops.h>
#include "searchtoolbar.h"
#include "searchenginelist.h"
#include "searchpluginsettings.h"
#include "searchplugin.h"

using namespace bt;

namespace kt
{

	SearchToolBar::SearchToolBar(SearchPlugin* plugin) : QObject(plugin)
	{
		KActionCollection* ac = plugin->actionCollection();
		
		m_search_text = new KComboBox((QWidget*)0);
		m_search_text->setEditable(true);
		m_search_text->setMaxCount(20);
		m_search_text->setInsertPolicy(QComboBox::NoInsert);
		m_search_text->setMinimumWidth(150);

		KLineEdit *search_text_lineedit = new KLineEdit(m_search_text);
		search_text_lineedit->setClearButtonShown(true);
		m_search_text->setLineEdit(search_text_lineedit);

		connect(m_search_text->lineEdit(),SIGNAL(returnPressed()),this,SLOT(searchBoxReturn()));
		connect(m_search_text,SIGNAL(textChanged(const QString &)),this,SLOT(textChanged( const QString& )));

		KAction* search_text_action = new KAction(i18n("Search Text"),this);
		search_text_action->setDefaultWidget(m_search_text);
		ac->addAction("search_text",search_text_action);

		m_search_new_tab = new KAction(KIcon("edit-find"),i18n("Search"),this);
		connect(m_search_new_tab,SIGNAL(triggered()),this,SLOT(searchNewTabPressed()));
		m_search_new_tab->setEnabled(false);
		ac->addAction("search",m_search_new_tab);
		
		KAction* search_engine_action = new KAction(i18n("Search Engine"),this);
		m_search_engine = new KComboBox((QWidget*)0);
		search_engine_action->setDefaultWidget(m_search_engine);
		ac->addAction("search_engine",search_engine_action);
		
		KAction* search_engine_label_action = new KAction(i18n("Search Engine Label"),this);
		QLabel* l = new QLabel(i18n(" Engine: "),(QWidget*)0);
		search_engine_label_action->setDefaultWidget(l);
		ac->addAction("search_engine_label",search_engine_label_action);
		
		loadSearchHistory();
		m_search_engine->setCurrentIndex(SearchPluginSettings::searchEngine());
	}

	SearchToolBar::~SearchToolBar()
	{
	}
	
	int SearchToolBar::currentSearchEngine() const
	{
		return m_search_engine->currentIndex();
	}
	
	void SearchToolBar::saveSettings()
	{
		SearchPluginSettings::setSearchEngine(m_search_engine->currentIndex());
		SearchPluginSettings::self()->writeConfig();
	}
	
	void SearchToolBar::updateSearchEngines(const SearchEngineList & sl)
	{
		int ci = 0;
		if (m_search_engine->count() == 0)
			ci = SearchPluginSettings::searchEngine();
		else
			ci = m_search_engine->currentIndex(); 
		
		m_search_engine->clear();
		for (Uint32 i = 0;i < sl.getNumEngines();i++)
		{
			m_search_engine->addItem(sl.getEngineName(i));
		}
		m_search_engine->setCurrentIndex(ci);
	}
	
	void SearchToolBar::searchBoxReturn()
	{
		QString str = m_search_text->currentText();
		KCompletion *comp = m_search_text->completionObject();
		if (!m_search_text->contains(str))
		{
			comp->addItem(str);
			m_search_text->addItem(str);
		}
		m_search_text->lineEdit()->clear();
		saveSearchHistory();
		search(str,m_search_engine->currentIndex(),SearchPluginSettings::openInExternal());
	}
	
	void SearchToolBar::searchNewTabPressed()
	{
		searchBoxReturn();
	}
	
	void SearchToolBar::textChanged(const QString & str)
	{
		m_search_new_tab->setEnabled(str.length() > 0);
	}

	void SearchToolBar::loadSearchHistory()
	{
		QFile fptr(kt::DataDir() + "search_history");
		if (!fptr.open(QIODevice::ReadOnly))
			return;
		
		KCompletion *comp = m_search_text->completionObject();
		
		Uint32 cnt = 0;
		QTextStream in(&fptr);
		while (!in.atEnd() && cnt < 50)
		{
			QString line = in.readLine();
			if (line.isNull())
				break; 
			
			if (!m_search_text->contains(line))
			{
				comp->addItem(line);
				m_search_text->addItem(line);
			}
			cnt++;
		}
		
		m_search_text->lineEdit()->clear();
	}
	
	void SearchToolBar::saveSearchHistory()
	{
		QFile fptr(kt::DataDir() + "search_history");
		if (!fptr.open(QIODevice::WriteOnly))
			return;
		
		QTextStream out(&fptr);
		KCompletion *comp = m_search_text->completionObject();
		QStringList items = comp->items();
		for (QStringList::iterator i = items.begin();i != items.end();i++)
		{
			out << *i << endl;
		}
	}
	
	void SearchToolBar::clearHistory()
	{
		bt::Delete(kt::DataDir() + "search_history");
		KCompletion *comp = m_search_text->completionObject();
		m_search_text->clear();
		comp->clear();
	}

}

#include "searchtoolbar.moc"

