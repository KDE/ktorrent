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

#include <kinputdialog.h>

#include <groups/groupmanager.h>

#include "filterdetails.h"
#include "filterconstants.h"
#include "capturecheckerdetails.h"

#include <util/log.h>

using namespace bt;

namespace kt
	{
	
	FilterDetails::FilterDetails(CoreInterface* core, QWidget * parent) : QDialog(parent), core(core)
		{
		setupUi(this);
		
		//setup defaults for drop down menus
		//type
		type->addItem("Reject Torrents", FT_REJECT);
		type->addItem("Accept Torrents", FT_ACCEPT);
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
		//make sure multimatch is correct for the torrent type setting
		onTypeChange(type->currentIndex());
		
		//captureCheck - get it's enabled state set correctly.
		onMultiMatchChange(multiMatch->currentIndex());
		
		//rerelease
		rerelease->addItem("Ignore Rereleases", RR_IGNORE);
		rerelease->addItem("Download All Rereleases", RR_DOWNLOAD_ALL);
		rerelease->addItem("Download First Rereleases", RR_DOWNLOAD_FIRST);
		onRereleaseChange(rerelease->currentIndex());
		
		//sourceListType
		sourceListType->addItem("Exclusive", SL_EXCLUSIVE);
		sourceListType->addItem("Inclusive", SL_INCLUSIVE);
		
		//multiMatchToolbar - for adding loading (and maybe saving) presets
		
		//expressionsToolbar - for adding and removing expressions
		expressionsToolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
		expressionsToolbar->setOrientation(Qt::Vertical);
		expressionsToolbar->setIconDimensions(16);
		expressionAdd = new KAction(KIcon("list-add"),i18n("Add Expression"),this);
		expressionRemove = new KAction(KIcon("list-remove"),i18n("Remove Expression"),this);
		expressionsToolbar->addAction(expressionAdd);
		expressionsToolbar->addAction(expressionRemove);
		expressionRemove->setEnabled(false);
		connect(expressionAdd, SIGNAL(triggered( bool )), this, SLOT(addExpression()));
		connect(expressionRemove, SIGNAL(triggered( bool )), this, SLOT(removeExpression()));
		connect(expressions, SIGNAL(itemSelectionChanged()), this, SLOT(onExpressionSelectionChange()));
		
		//sourceListToolbar - for adding and removing sources
		sourceListToolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
		sourceListToolbar->setOrientation(Qt::Vertical);
		sourceListToolbar->setIconDimensions(16);
		sourceAdd = new KActionMenu(KIcon("list-add"),i18n("Add Source"),this);
		sourceAdd->setDelayed(false);
		sourceRemove = new KAction(KIcon("list-remove"),i18n("Remove Source"),this);
		sourceRemove->setEnabled(false);
		sourceListToolbar->addAction(sourceAdd);
		sourceListToolbar->addAction(sourceRemove);
		
		//matchesToolbar - for deleting matches so they're redownloaded
		//		perhaps also add reprocessing button in here.
		
		//hook up signals to push out the relevant information
		connect(name, SIGNAL(textChanged( const QString& )), this, SIGNAL(nameChanged(const QString&)));
		connect(type, SIGNAL(currentIndexChanged( int )), this, SLOT(emitType()));
		connect(group, SIGNAL(currentIndexChanged( const QString& )), this, SIGNAL(groupChanged(const QString&)));
		connect(expressions, SIGNAL(itemChanged( QListWidgetItem* )), this, SLOT(emitExpressions()));
		connect(sourceListType, SIGNAL(currentIndexChanged( int )), this, SLOT(emitSourceListType()));
		connect(sourceList, SIGNAL(itemChanged( QListWidgetItem* )), this, SLOT(emitSourceList()));
		connect(multiMatch, SIGNAL(currentIndexChanged( int )), this, SLOT(emitMultiMatch()));
		connect(rerelease, SIGNAL(currentIndexChanged( int )), this, SLOT(emitRerelease()));
		connect(rereleaseTerms, SIGNAL(textChanged( const QString& )), this, SIGNAL(rereleaseTermsChanged(const QString&)));
		
		}
	
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
		onRereleaseChange(rerelease->currentIndex());
		captureChecker->resizeColumns();
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
	
	void FilterDetails::onRereleaseChange(int curIndex)
		{
		rereleaseTerms->setEnabled(rerelease->itemData(curIndex) != RR_IGNORE);
		}
		
	void FilterDetails::refreshSizes()
		{
		captureChecker->resizeColumns();
		}
	
	void FilterDetails::connectFilter(Filter * value)
		{
		//set all the values
		name->setText(value->getName());
		setType(value->getType());
		onTypeChange(type->currentIndex());
		group->setCurrentIndex(group->findText(value->getGroup()));
		expressions->clear();
		expressions->addItems(value->getExpressions());
		setSourceListType(value->getSourceListType());
		sourceList->clear();
		sourceList->addItems(value->getSourceList());
		setMultiMatch(value->getMultiMatch());
		onMultiMatchChange(multiMatch->currentIndex());
		setRerelease(value->getRerelease());
		onRereleaseChange(rerelease->currentIndex());
		rereleaseTerms->setText(value->getRereleaseTerms());
		captureChecker->setCaptureChecker(value->getCaptureChecker());
		
		connect(testString, SIGNAL(textChanged( const QString& )), captureChecker, SLOT(setTestString(const QString&)));
		connect(testString, SIGNAL(textChanged( const QString& )), this, SLOT(checkTestString()));
		
		connect(this, SIGNAL(multiMatchChanged(int)), this, SLOT(checkTestString()));
		connect(this, SIGNAL(rereleaseChanged(int)), this, SLOT(checkTestString()));
		connect(this, SIGNAL(expressionsChanged(QStringList)), this, SLOT(checkTestString()));
		connect(captureChecker, SIGNAL(capturesChanged(QMap< QString, QString >)), this, SLOT(checkTestString()));
		connect(captureChecker, SIGNAL(variablesChanged(QList< Variable >)), this, SLOT(checkTestString()));
		connect(captureChecker, SIGNAL(mappingsChanged()), this, SLOT(checkTestString()));
		
		//not actually filter related, but let's us set data before things start updating
		connect(type, SIGNAL(currentIndexChanged( int )), this, SLOT(onTypeChange(int)));
		connect(multiMatch, SIGNAL(currentIndexChanged( int )), this, SLOT(onMultiMatchChange(int)));
		connect(rerelease, SIGNAL(currentIndexChanged( int )), this, SLOT(onRereleaseChange(int)));
		
		//connect the signals to push changes to the value
		connect(this, SIGNAL(nameChanged(const QString&)), value, SLOT(setName(const QString&)));
		connect(this, SIGNAL(typeChanged(int)), value, SLOT(setType(int)));
		connect(this, SIGNAL(groupChanged(const QString&)), value, SLOT(setGroup(const QString&)));
		connect(this, SIGNAL(expressionsChanged(QStringList)), value, SLOT(setExpressions(QStringList)));
		connect(this, SIGNAL(sourceListTypeChanged(int)), value, SLOT(setSourceListType(int)));
		connect(this, SIGNAL(sourceListChanged(QStringList)), value, SLOT(setSourceList(QStringList)));
		connect(this, SIGNAL(multiMatchChanged(int)), value, SLOT(setMultiMatch(int)));
		connect(this, SIGNAL(rereleaseChanged(int)), value, SLOT(setRerelease(int)));
		connect(this, SIGNAL(rereleaseTermsChanged(const QString&)), value, SLOT(setRereleaseTerms(const QString&)));
		
		}
		
	void FilterDetails::setFilter(Filter * value)
		{
		if (value == filter)
			return;
		
		//we only want to disconnect it if it's not null
		if (filter)
			disconnect(filter);
		
		//we don't want to connect it up and try call functions if the new one is null
		if (value)
			connectFilter(value);
		
		filter = value;
		}
	
	Filter* FilterDetails::getFilter()
		{
		return filter;
		}
	
	void FilterDetails::emitType()
		{
		//it's the userdata our filter cares about and there's no signal for that
		emit typeChanged(type->itemData(type->currentIndex()).toInt());
		}
	
	void FilterDetails::emitExpressions()
		{
		//for taking changes to the expressions list and pushing them over to the filter
		QStringList value;
		
		for (int i=0; i<expressions->count(); i++)
			{
			value << expressions->item(i)->text();
			}
		
		Out(SYS_BTF|LOG_DEBUG) << "Emitting expressions changed to: " << value.join(" ") << endl;
		
		emit expressionsChanged(value);
		}
		
	void FilterDetails::emitSourceListType()
		{
		//it's the userdata our filter cares about and there's no signal for that
		emit sourceListTypeChanged(sourceListType->itemData(sourceListType->currentIndex()).toInt());
		}
	
	void FilterDetails::emitSourceList()
		{
		//for taking changes to the sources list and pushing them over to the filter
		QStringList values;
		
		for (int i=0; i<sourceList->count(); i++)
			{
			values << sourceList->item(i)->text();
			}
		
		emit sourceListChanged(values);
		}

	void FilterDetails::emitMultiMatch()
		{
		//it's the userdata our filter cares about and there's no signal for that
		emit multiMatchChanged(multiMatch->itemData(multiMatch->currentIndex()).toInt());
		}
	
	void FilterDetails::emitRerelease()
		{
		//it's the userdata our filter cares about and there's no signal for that
		emit rereleaseChanged(rerelease->itemData(rerelease->currentIndex()).toInt());
		}
	
	void FilterDetails::setType(int value)
		{
		for (int i=0; i<type->count(); i++)
			{
			if (value == type->itemData(i))
				{
				type->setCurrentIndex(i);
				return;
				}
			}
		}
	
	void FilterDetails::setSourceListType(int value)
		{
		for (int i=0; i<sourceListType->count(); i++)
			{
			if (value == sourceListType->itemData(i))
				{
				sourceListType->setCurrentIndex(i);
				return;
				}
			}
		}
	
	void FilterDetails::setExpressions(QStringList value)
		{
		expressions->clear();
		expressions->addItems(value);
		
		for (int i=0; i<expressions->count(); i++)
			{
			expressions->item(i)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled);
			}
		}
		
	void FilterDetails::setMultiMatch(int value)
		{
		for (int i=0; i<multiMatch->count(); i++)
			{
			if (value == multiMatch->itemData(i))
				{
				multiMatch->setCurrentIndex(i);
				return;
				}
			}
		
		checkTestString();
		}
	
	void FilterDetails::setRerelease(int value)
		{
		for (int i=0; i<rerelease->count(); i++)
			{
			if (value == rerelease->itemData(i))
				{
				rerelease->setCurrentIndex(i);
				return;
				}
			}
			
		checkTestString();
		}
	
	void FilterDetails::addExpression()
		{
		bool ok = false;
		QString name = KInputDialog::getText(i18n("Add Expression"), 
					i18n("Please enter the expression."),QString(),&ok,this);
		
		if (ok)
			{
			if (filter)
				{
				expressions->addItem(name);
				expressions->item(expressions->count()-1)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled);
				}
			}
		}
	
	void FilterDetails::removeExpression()
		{
		if (expressions->selectedItems().count())
			expressions->removeItemWidget(expressions->selectedItems().at(0));
		}
	
	void FilterDetails::onExpressionSelectionChange()
		{
		expressionRemove->setEnabled(expressions->selectedItems().count());
		}
	
	void FilterDetails::checkTestString()
		{
		if (!filter)
			return;
		
		if (testString->text().isEmpty())
			{
			//it's empty so let's do no more and reset to default background colour
			testString->setPalette(name->palette());
			return;
			}
		
		QPalette curPalette = testString->palette();
		
		if (filter->checkMatch(testString->text()))
			{
			//it's a match make it green
			curPalette.setBrush(QPalette::Base, QBrush(QColor(0,255,0,128)));
			}
		else
			{
			//no match - make it red
			curPalette.setBrush(QPalette::Base, QBrush(QColor(255,0,0,128)));
			}
		
		testString->setPalette(curPalette);
		}
	
	}
