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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include <qfile.h>
#include <qtextstream.h>
#include <qapplication.h>
#include <qlistbox.h>
#include <qcheckbox.h>
#include <kglobal.h>
#include <kpushbutton.h>
#include <kiconloader.h>
#include <kcombobox.h>
#include <kcompletion.h>
#include <qlabel.h>
#include <klocale.h>
#include "searchtab.h"
#include "searchenginelist.h"
#include "searchpluginsettings.h"
#include "functions.h"

using namespace bt;

namespace kt
{

	SearchTab::SearchTab(KToolBar* tb) : m_tool_bar(tb)
	{
		m_search_text = new KComboBox(tb);
		m_search_text->setEditable(true);
		
		m_clear_button = new KPushButton(tb);
		m_search_new_tab = new KPushButton(i18n("Search"),tb);
		m_search_engine = new KComboBox(tb);
		
		m_clear_button->setIconSet(SmallIconSet(QApplication::reverseLayout() ? "clear_left" : "locationbar_erase"));
		m_clear_button->setEnabled(false);

		connect(m_search_new_tab,SIGNAL(clicked()),this,SLOT(searchNewTabPressed()));
		connect(m_search_text,SIGNAL(returnPressed(const QString&)),this,SLOT(searchBoxReturn( const QString& )));
		connect(m_search_text,SIGNAL(textChanged(const QString &)),this,SLOT(textChanged( const QString& )));
		connect(m_clear_button,SIGNAL(clicked()),this,SLOT(clearButtonPressed()));
		m_search_text->setMaxCount(20);
		m_search_new_tab->setEnabled(false);
		m_search_text->setInsertionPolicy(QComboBox::NoInsertion);
		
		tb->insertWidget(1,-1,m_clear_button);
		tb->insertWidget(2,-1,m_search_text);
		tb->insertWidget(3,-1,m_search_new_tab);
		tb->insertWidget(4,-1,new QLabel(i18n(" Engine: "),tb));
		tb->insertWidget(5,-1,m_search_engine);
		loadSearchHistory();
	}

	SearchTab::~SearchTab()
	{
	}
	
	void SearchTab::saveSettings()
	{
		SearchPluginSettings::setSearchEngine(m_search_engine->currentItem());
		SearchPluginSettings::writeConfig();
	}
	
	void SearchTab::updateSearchEngines(const SearchEngineList & sl)
	{
		int ci = 0;
		if (m_search_engine->count() == 0)
			ci = SearchPluginSettings::searchEngine();
		else
			ci = m_search_engine->currentItem(); 
		
		m_search_engine->clear();
		for (Uint32 i = 0;i < sl.getNumEngines();i++)
		{
			m_search_engine->insertItem(sl.getEngineName(i));
		}
		m_search_engine->setCurrentItem(ci);
	}
	
	void SearchTab::searchBoxReturn(const QString & str)
	{
		KCompletion *comp = m_search_text->completionObject();
		if (!m_search_text->contains(str))
		{
			comp->addItem(str);
			m_search_text->insertItem(str);
		}
		m_search_text->clearEdit();
		saveSearchHistory();
		search(str,m_search_engine->currentItem(),SearchPluginSettings::openInExternal());
	}
	
	void SearchTab::clearButtonPressed()
	{
		m_search_text->clearEdit();
	}
	
	void SearchTab::searchNewTabPressed()
	{
		searchBoxReturn(m_search_text->currentText());
	}
	
	void SearchTab::textChanged(const QString & str)
	{
		m_search_new_tab->setEnabled(str.length() > 0);
		m_clear_button->setEnabled(str.length() > 0);
	}

	void SearchTab::loadSearchHistory()
	{
		QFile fptr(kt::DataDir() + "search_history");
		if (!fptr.open(IO_ReadOnly))
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
				m_search_text->insertItem(line);
			}
			cnt++;
		}
		
		m_search_text->clearEdit();
	}
	
	void SearchTab::saveSearchHistory()
	{
		QFile fptr(kt::DataDir() + "search_history");
		if (!fptr.open(IO_WriteOnly))
			return;
		
		QTextStream out(&fptr);
		KCompletion *comp = m_search_text->completionObject();
		QStringList items = comp->items();
		for (QStringList::iterator i = items.begin();i != items.end();i++)
		{
			out << *i << endl;
		}
	}
}

#include "searchtab.moc"

