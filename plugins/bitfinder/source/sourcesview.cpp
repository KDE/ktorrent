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

#include <QVBoxLayout>

#include "sourcesview.h"

namespace kt
	{
	
	SourcesView::SourcesView(QWidget * parent) : QWidget(parent)
		{
		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->setSpacing(0);
		layout->setMargin(0);
		
		toolBar = new KToolBar(this);
		toolBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
		layout->addWidget(toolBar);
		
		sourcesList = new QListView(this);
		//sourcesList->setModel(model);
		layout->addWidget(sourcesList);
		
		connect(sourcesList,SIGNAL(doubleClicked(const QModelIndex &)),this,SIGNAL(doubleClicked(const QModelIndex&)));
		}
		
	}