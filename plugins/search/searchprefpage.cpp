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
#include <kurl.h>
#include <qtooltip.h>
#include <qfile.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kactivelabel.h>
#include <kpushbutton.h>
#include <klineedit.h>
#include <kmessagebox.h>
#include <kio/copyjob.h>
#include <kio/jobuidelegate.h>
#include <klineedit.h>
#include <kinputdialog.h>

#include <qlabel.h>
#include <qcheckbox.h>
#include <qradiobutton.h>

#include <util/log.h>
#include <util/constants.h>
#include <util/functions.h>
#include <util/fileops.h>
#include <util/error.h>
#include <interfaces/functions.h>
#include "searchprefpage.h"
#include "searchplugin.h"
#include "searchenginelist.h"
#include "searchpluginsettings.h"
#include "opensearchdownloadjob.h"

using namespace bt;

namespace kt
{
	SearchPrefPage::SearchPrefPage(SearchPlugin* plugin,SearchEngineList* sl,QWidget* parent)
		: PrefPageInterface(SearchPluginSettings::self(),i18n("Search"), "edit-find",parent), plugin(plugin),engines(sl)
	{
		setupUi(this);
		m_engines->setModel(sl);
		
		connect(m_add, SIGNAL(clicked()), this, SLOT(addClicked()));
		connect(m_remove, SIGNAL(clicked()), this, SLOT(removeClicked()));
		connect(m_add_default, SIGNAL(clicked()), this, SLOT(addDefaultClicked()));
		connect(m_remove_all, SIGNAL(clicked()), this, SLOT(removeAllClicked()));
		connect(m_clear_history,SIGNAL(clicked()),this,SLOT(clearHistory()));
		connect(m_engines->selectionModel(),SIGNAL(selectionChanged(const QItemSelection & ,const QItemSelection & )),
				this,SLOT(selectionChanged(const QItemSelection&, const QItemSelection&)));
		
		connect(kcfg_useCustomBrowser, SIGNAL(toggled(bool)), this, SLOT(customToggled( bool )));
		connect(kcfg_openInExternal,SIGNAL(toggled(bool)),this, SLOT(openInExternalToggled(bool)));
		QButtonGroup* bg = new QButtonGroup(this);
		bg->addButton(kcfg_useCustomBrowser);
		bg->addButton(kcfg_useDefaultBrowser);
		
		m_remove_all->setEnabled(sl->rowCount(QModelIndex()) > 0);
		m_remove->setEnabled(false);
	}


	SearchPrefPage::~SearchPrefPage()
	{}
	
	void SearchPrefPage::selectionChanged(const QItemSelection & selected,const QItemSelection & deselected)
	{
		Q_UNUSED(deselected);
		m_remove->setEnabled(selected.count() > 0);
	}
	
	void SearchPrefPage::loadSettings()
	{
		openInExternalToggled(SearchPluginSettings::openInExternal());
	}

	void SearchPrefPage::loadDefaults()
	{
		loadSettings();
	}
  
	void SearchPrefPage::addClicked()
	{
		bool ok = false;
		QString name = KInputDialog::getText(i18n("Add a Search Engine"), 
						i18n("Enter the hostname of the search engine (for example www.google.com) :"),QString(),&ok,this);
		if (!ok || name.isEmpty())
			return;
		
		if (!name.startsWith("http://") || !name.startsWith("https://"))
			name = "http://" + name;
		
		KUrl url(name);
		QString dir = kt::DataDir() + "searchengines/" + url.host();
		int idx = 1;
		while (bt::Exists(dir))
		{
			dir += QString::number(idx++);
		}
		
		dir += "/";
		
		try
		{
			bt::MakeDir(dir,false);
		}
		catch (bt::Error & err)
		{
			KMessageBox::error(this,err.toString());
			return;
		}
		
		OpenSearchDownloadJob* j = new OpenSearchDownloadJob(url,dir);
		connect(j,SIGNAL(result(KJob*)),this,SLOT(downloadJobFinished(KJob*)));
		j->start();
	}
	
	void SearchPrefPage::downloadJobFinished(KJob* j)
	{
		OpenSearchDownloadJob* osdj = (OpenSearchDownloadJob*)j;
		if (osdj->error())
		{
			bool ok = false;
			QString msg = i18n("Opensearch is not supported by %1, you will need to enter the search URL manually. "
					"The URL should contain {searchTerms}, ktorrent will replace this by the thing you are searching for.",osdj->hostname());
			QString url = KInputDialog::getText(i18n("Add a Search Engine"),msg,QString(),&ok,this);
			if (ok && !url.isEmpty())
			{
				if (!url.contains("{searchTerms}"))
				{
					KMessageBox::error(this,i18n("The URL %s, does not contain {searchTerms}.",url));
				}
				else
				{
					try
					{
						engines->addEngine(osdj->directory(),url);
					}
					catch (bt::Error & err)
					{
						KMessageBox::error(this,err.toString());
						bt::Delete(osdj->directory(),true);
					}
				}
			}
		}
		else
		{
			engines->addEngine(osdj);
		}
	}
 
	void SearchPrefPage::removeClicked()
	{
		QModelIndexList sel = m_engines->selectionModel()->selectedRows();
		engines->removeEngines(sel);
		m_remove_all->setEnabled(engines->rowCount(QModelIndex()) > 0);
		m_remove->setEnabled(m_engines->selectionModel()->selectedRows().count() > 0);
	}
 
	void SearchPrefPage::addDefaultClicked()
	{
		engines->addDefaults();
		m_remove_all->setEnabled(engines->rowCount(QModelIndex()) > 0);
		m_remove->setEnabled(m_engines->selectionModel()->selectedRows().count() > 0);
	}
 
	void SearchPrefPage::removeAllClicked()
	{
		engines->removeAllEngines();	
		m_remove_all->setEnabled(engines->rowCount(QModelIndex()) > 0);
		m_remove->setEnabled(m_engines->selectionModel()->selectedRows().count() > 0);
	}
	
	void SearchPrefPage::customToggled(bool toggled)
	{
		kcfg_customBrowser->setEnabled(toggled);
	}

	void SearchPrefPage::openInExternalToggled(bool on)
	{
		if (on)
		{
			kcfg_useCustomBrowser->setEnabled(true);
			kcfg_customBrowser->setEnabled(SearchPluginSettings::useCustomBrowser());
			kcfg_useDefaultBrowser->setEnabled(true);
		}
		else
		{
			kcfg_useCustomBrowser->setEnabled(false);
			kcfg_customBrowser->setEnabled(false);
			kcfg_useDefaultBrowser->setEnabled(false);
		}
	}

	void SearchPrefPage::clearHistory()
	{
		emit clearSearchHistory();
	}
}

#include "searchprefpage.moc"
