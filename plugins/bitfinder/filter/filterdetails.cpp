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

#include <groups/groupmanager.h>

#include "filterdetails.h"
#include "filterconstants.h"
#include "capturecheckerdetails.h"

#include <util/log.h>

using namespace bt;

namespace kt
	{
	void FilterDetails::updateGroupList(QString oldName, QString newName)
		{
		QString curItem = group->currentText();
		if (curItem == oldName)
			{
			if (!newName.isEmpty())
				curItem = newName;
			}
		
		group->clear();
		group->addItem("Ungrouped");
		group->addItems(core->getGroupManager()->customGroupNames());
		int item=0;
		for (int i=0; i<group->count(); i++)
			{
			if (group->itemText(i) == curItem)
				{
				item = i;
				break;
				}
			}
		group->setCurrentIndex(item);
		}
	
	void FilterDetails::onMultiMatchChange(int curIndex)
		{
		captureChecker->setEnabled(multiMatch->itemData(curIndex) == MM_CAPTURE_CHECKING);
		rerelease->setEnabled(multiMatch->itemData(curIndex) != MM_ALWAYS_MATCH);
		}
	
	void FilterDetails::onTypeChange(int curIndex)
		{
		if (type->itemData(curIndex) == FT_REJECT)
			{
			if (multiMatch->itemData(0) == MM_ONCE_ONLY)
				{
				multiMatch->removeItem(0);
				}
			}
		else
			{
			if (multiMatch->itemData(0) != MM_ONCE_ONLY)
				{
				multiMatch->insertItem(0, "Match once only", MM_ONCE_ONLY);
				}
			}
		}
	
	FilterDetails::FilterDetails(CoreInterface* core, QWidget * parent) : QDialog(parent), core(core)
		{
		setupUi(this);
		
		//setup defaults for drop down menus
		//type
		type->addItem("Reject Torrents", FT_REJECT);
		type->addItem("Accept Torrents", FT_ACCEPT);
		connect(type, SIGNAL(currentIndexChanged( int )), this, SLOT(onTypeChange(int)));
		//group
		updateGroupList();
		//I've hooked this up to the UI for now, but will probably want to have this connected
		//	directly to filters later on. Otherwise renaming a group will only update when
		//	the property page is open.
		connect(core->getGroupManager(), SIGNAL(customGroupsChanged(QString, QString)), 
					this, SLOT(updateGroupList(QString, QString)));
		//multiMatch
		multiMatch->addItem("Match only once", MM_ONCE_ONLY);
		multiMatch->addItem("Match every time", MM_ALWAYS_MATCH);
		multiMatch->addItem("Use Capture Checking", MM_CAPTURE_CHECKING);
		connect(multiMatch, SIGNAL(currentIndexChanged( int )), this, SLOT(onMultiMatchChange(int)));
		//rerelease
		rerelease->addItem("Ignore Rereleases", RR_IGNORE);
		rerelease->addItem("Download All Rereleases", RR_DOWNLOAD_ALL);
		rerelease->addItem("Download First Rereleases", RR_DOWNLOAD_FIRST);
		//sourceListType
		sourceListType->addItem("Exclusive", SL_EXCLUSIVE);
		sourceListType->addItem("Inclusive", SL_INCLUSIVE);
		
		//multiMatchToolbar - for adding loading (and maybe saving) presets
		
		//captureCheck - get it's enabled state set correctly.
		onMultiMatchChange(multiMatch->currentIndex());
		
		//sourceListToolbar - for adding and removing sources
		sourceListToolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
		sourceListToolbar->setOrientation(Qt::Vertical);
		sourceListToolbar->setIconDimensions(16);
		sourceAdd = new KActionMenu(KIcon("list-add"),i18n("Add Source"),this);
		sourceAdd->setDelayed(false);
		sourceRemove = new KAction(KIcon("list-remove"),i18n("Remove Source"),this);
		sourceListToolbar->addAction(sourceAdd);
		sourceListToolbar->addAction(sourceRemove);
		
		//matchesToolbar - for deleting matches so they're redownloaded
		//		perhaps also add reprocessing button in here.
		}
	
	}