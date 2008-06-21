/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
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
#include <QVBoxLayout>
#include <util/log.h>
#include <ktoolbar.h>
#include <kactioncollection.h>
#include <kross/core/manager.h>
#include "scriptmanager.h"
#include "scriptmodel.h"

using namespace Kross;
using namespace bt;

namespace kt
{

	ScriptManager::ScriptManager(KActionCollection* ac,QWidget* parent)
			: QWidget(parent)
	{
		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->setSpacing(0);
		layout->setMargin(0);
		
		QAction* remove = ac->action("remove_script");
		toolbar = new KToolBar(this);
		toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
		layout->addWidget(toolbar);
		toolbar->addAction(ac->action("add_script"));
		toolbar->addAction(remove);
		connect(this,SIGNAL(enableRemoveScript(bool)),remove,SLOT(setEnabled(bool)));
		remove->setEnabled(false);
		
		view = new QListView(this);
		layout->addWidget(view);
	
		model = new ScriptModel(Kross::Manager::self().actionCollection(),this);
 		view->setModel(model);
		
		connect(view->selectionModel(),SIGNAL(selectionChanged(const QItemSelection &,const QItemSelection)),
				this,SLOT(onSelectionChanged(const QItemSelection &,const QItemSelection)));
	}


	ScriptManager::~ScriptManager()
	{
	}

	void ScriptManager::onSelectionChanged(const QItemSelection & selected,const QItemSelection & deselected)
	{
		Q_UNUSED(deselected);
		enableRemoveScript(selected.count() > 0);
	}

}
