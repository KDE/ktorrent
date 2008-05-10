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
#include <ktoolbar.h>
#include <kinputdialog.h>
#include <klocale.h>
#include <kactioncollection.h>

#include <QVBoxLayout>

#include "filtersview.h"

namespace kt
	{
	
	FiltersView::FiltersView(FilterListModel* model, QWidget * parent) : QWidget(parent),filterListModel(model)
		{
		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->setSpacing(0);
		layout->setMargin(0);
		
		toolBar = new KToolBar(this);
 		toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
		layout->addWidget(toolBar);
		
		filtersList = new QListView(this);
		filtersList->setModel(filterListModel);
		layout->addWidget(filtersList);
		
		connect(filtersList,SIGNAL(doubleClicked(const QModelIndex &)),this,SIGNAL(doubleClicked(const QModelIndex&)));
		connect(filtersList,SIGNAL(doubleClicked(const QModelIndex &)),filterListModel, SLOT(openFilterTab(const QModelIndex&)));
		
		setupFiltersActions();
		}
	
	void FiltersView::setupFiltersActions()
		{
		//create the Action for adding sources
		addFilter = new KAction(KIcon("list-add"),i18n("Add Filter"),this);
		
		//create the Action for removing sources
		removeFilter = new KAction(KIcon("list-remove"), i18n("Remove Filter"), this);
		
		//create the Action for moving a filter up
		filterUp = new KAction(KIcon("arrow-up"), i18n("Move Filter Up"), this);
		
		//create the Action for moving a filter up
		filterDown = new KAction(KIcon("arrow-down"), i18n("Move Filter Down"), this);

		connect(addFilter, SIGNAL(triggered(bool)), this, SLOT(addNewFilter()));
		connect(removeFilter, SIGNAL(triggered(bool)), this, SLOT(removeFilters()));
		
		toolBar->addAction(addFilter);
		toolBar->addAction(removeFilter);
		toolBar->addAction(filterUp);
		toolBar->addAction(filterDown);
		}
	
	void FiltersView::addNewFilter()
		{
		bool ok = false;
		QString name = KInputDialog::getText(i18n("Add New Filter"), 
					i18n("Please enter the new filter name."),QString(),&ok,this);
		
		if (ok)
			{
			if (filterListModel)
				filterListModel->addNewFilter(name);
			}
		}
	
	void FiltersView::removeFilters()
		{
		QModelIndexList rows = filtersList->selectionModel()->selectedRows();
		
		for (int i=0; i < rows.count(); i++)
			{
			filterListModel->removeFilter(rows.at(i));
			}
		}
	
	}