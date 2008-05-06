/***************************************************************************
 *   Copyright (C) 2008 by Alan Jones                                      *
 *   skyphyr@gmail.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
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

#include <kgenericfactory.h>
#include <kicon.h>
#include <klocale.h>
#include <kactioncollection.h>

#include <util/log.h>
//Out(SYS_BTF|LOG_DEBUG) << "Debug messages look like this - Place them somewhere useful" << endl;

#include "bitfinderplugin.h"

#include "filter/capturecheckerdetails.h"

K_EXPORT_COMPONENT_FACTORY (ktbitfinderplugin, KGenericFactory<kt::BitFinderPlugin> ("ktbitfinderplugin"))

using namespace bt;

namespace kt
	{

	BitFinderPlugin::BitFinderPlugin (QObject* parent, const QStringList& args) : Plugin (parent)
		{
		Q_UNUSED (args);
		}


	BitFinderPlugin::~BitFinderPlugin()
		{
		
		}

	void BitFinderPlugin::setupSourcesActions()
		{
		KActionCollection* ac = actionCollection();
		
		//create the Action for adding sources
		addSourceMenu = new KActionMenu(KIcon("list-add"),i18n("Add Source"),this);
		addSourceMenu->setDelayed(false);
		
		//create the Action for removing sources
		removeSource = new KAction(KIcon("list-remove"), i18n("Remove Source"), this);
		
		//add the RSS Source Action to the menu
		addRssSource = new KAction(KIcon("application-rss+xml"), i18n("RSS Feed"), this);
		addSourceMenu->addAction(addRssSource);
		
		ac->addAction("Add Rss Source",addRssSource);
		ac->addAction("Remove Source",removeSource);
		
		}

	void BitFinderPlugin::setupFiltersActions()
		{
		KActionCollection* ac = actionCollection();
		
		//create the Action for adding sources
		addFilter = new KAction(KIcon("list-add"),i18n("Add Filter"),this);
		
		//create the Action for removing sources
		removeFilter = new KAction(KIcon("list-remove"), i18n("Remove Filter"), this);
		
		//create the Action for moving a filter up
		filterUp = new KAction(KIcon("arrow-up"), i18n("Move Filter Up"), this);
		
		//create the Action for moving a filter up
		filterDown = new KAction(KIcon("arrow-down"), i18n("Move Filter Down"), this);

		//put the actions into the collection
		ac->addAction("Add Filter",addFilter);
		ac->addAction("Remove Filter",removeFilter);
		ac->addAction("Move Filter Up",filterUp);
		ac->addAction("Move Filter Down", filterDown);
		
		}

	void BitFinderPlugin::load()
		{
		//Add the BF Sources Menu on the left dock
		sourcesView = new SourcesView();
		getGUI()->addToolWidget(sourcesView,"ktorrent",i18n("BF Sources"),GUIInterface::DOCK_LEFT);
		
		setupSourcesActions();
		
		QToolBar* tb = sourcesView->sourcesToolBar();
		tb->addAction(addSourceMenu);
		tb->addAction(removeSource);
		
		//Build the filter model
		filterListModel = new FilterListModel(getCore(), getGUI(), this);
		
		//Add the BF Filters Menu on the left dock
		filtersView = new FiltersView(filterListModel);
		getGUI()->addToolWidget(filtersView,"view-filter",i18n("BF Filters"),GUIInterface::DOCK_LEFT);
		
		setupFiltersActions();
		
		tb = filtersView->filtersToolBar();
		tb->addAction(addFilter);
		tb->addAction(removeFilter);
		tb->addAction(filterUp);
		tb->addAction(filterDown);
		
		//add a filter tab in during early testing
		FilterDetails * testFilterTab = new FilterDetails(getCore());
		getGUI()->addTabPage(testFilterTab,"news-subscribe","Test Filter Tab",this);
		testFilterTab->refreshSizes();
		testFilterTab->setFilter(new Filter());
		
		filterDetailsList.append(testFilterTab);
		}

	void BitFinderPlugin::unload()
		{
		//remove the BF Sources Menu
		getGUI()->removeToolWidget(sourcesView);
		delete sourcesView;
		sourcesView = 0;
		
		
		getGUI()->removeToolWidget(filtersView);
		delete filtersView;
		filtersView = 0;
		}

	bool BitFinderPlugin::versionCheck (const QString& version) const
		{
		return version == KT_VERSION_MACRO;
		}

	void BitFinderPlugin::tabCloseRequest (kt::GUIInterface* gui, QWidget* tab)
		{
		//Check through the list of Source tabs
		
		//Check through the list of Filter tabs
		foreach(FilterDetails * filterTab, filterDetailsList)
			{
			if (filterTab == tab)
				{
				getGUI()->removeTabPage(filterTab);
				delete filterTab;
				}
			
			}
		
		}

	}