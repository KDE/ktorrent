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
#include <QAction>
#include <QVBoxLayout>
#include <util/log.h>
#include <kmenu.h>
#include <ktoolbar.h>
#include <kactioncollection.h>
#include <kross/core/manager.h>
#include "scriptmanager.h"
#include "scriptmodel.h"
#include "script.h"

using namespace Kross;
using namespace bt;

namespace kt
{

	ScriptManager::ScriptManager(ScriptModel* model,KActionCollection* ac,QWidget* parent)
			: QWidget(parent),model(model)
	{
		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->setSpacing(0);
		layout->setMargin(0);
		
		QAction* remove = ac->action("remove_script");
		QAction* add = ac->action("add_script");
		QAction* run = ac->action("run_script");
		QAction* stop = ac->action("stop_script");
		QAction* edit = ac->action("edit_script");
		QAction* properties = ac->action("script_properties");
		QAction* configure = ac->action("configure_script");
		
		toolbar = new KToolBar(this);
		toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
		layout->addWidget(toolbar);
		toolbar->addAction(add);
		toolbar->addAction(remove);
		toolbar->addAction(run);
		toolbar->addAction(stop);
		connect(this,SIGNAL(enableRemoveScript(bool)),remove,SLOT(setEnabled(bool)));
		connect(this,SIGNAL(enableRemoveScript(bool)),edit,SLOT(setEnabled(bool)));
		connect(this,SIGNAL(enableStopScript(bool)),stop,SLOT(setEnabled(bool)));
		connect(this,SIGNAL(enableRunScript(bool)),run,SLOT(setEnabled(bool)));
		connect(this,SIGNAL(enableProperties(bool)),properties,SLOT(setEnabled(bool)));
		connect(this,SIGNAL(enableConfigure(bool)),configure,SLOT(setEnabled(bool)));
		remove->setEnabled(false);
		
		view = new QListView(this);
		layout->addWidget(view);
	
 		view->setModel(model);
		view->setContextMenuPolicy(Qt::CustomContextMenu);
		view->setSelectionMode(QAbstractItemView::ExtendedSelection);
		view->setSelectionBehavior(QAbstractItemView::SelectRows);
		
		connect(view->selectionModel(),SIGNAL(selectionChanged(const QItemSelection &,const QItemSelection)),
				this,SLOT(onSelectionChanged(const QItemSelection &,const QItemSelection)));
		
		connect(view,SIGNAL(customContextMenuRequested(const QPoint & )),
				this,SLOT(showContextMenu(const QPoint& )));
		
		connect(model,SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
				this,SLOT(dataChanged(const QModelIndex&, const QModelIndex&)));
		
		context_menu = new KMenu(this);
		context_menu->addAction(add);
		context_menu->addAction(remove);
		context_menu->addSeparator();
		context_menu->addAction(run);
		context_menu->addAction(stop);
		context_menu->addSeparator();
		context_menu->addAction(edit);
		context_menu->addSeparator();
		context_menu->addAction(properties);
		context_menu->addAction(configure);
		
		add->setEnabled(true);
		remove->setEnabled(false);
		run->setEnabled(false);
		stop->setEnabled(false);
		edit->setEnabled(false);
		properties->setEnabled(false);
		configure->setEnabled(false);
	}


	ScriptManager::~ScriptManager()
	{
	}

	void ScriptManager::onSelectionChanged(const QItemSelection & selected,const QItemSelection & deselected)
	{
		Q_UNUSED(deselected);
		Q_UNUSED(selected);
		updateActions(selectedScripts());
	}
	
	void ScriptManager::updateActions(const QModelIndexList & selected)
	{
		enableRemoveScript(selected.count() > 0);
		int num_running = 0;
		int num_not_running = 0;
		foreach (const QModelIndex & idx,selected)
		{
			Script* s = model->scriptForIndex(idx);
			if (s && s->running())
				num_running++;
			else
				num_not_running++;
		}
		
		enableRunScript(selected.count() > 0 && num_not_running > 0);
		enableStopScript(selected.count() > 0 && num_running > 0);
		Script* s = model->scriptForIndex(selected.front());
		enableProperties(selected.count() == 1 && s && s->metaInfo().valid());
		enableConfigure(selected.count() == 1 && s && s->hasConfigure());
	}
	
	QModelIndexList ScriptManager::selectedScripts()
	{
		return view->selectionModel()->selectedRows();
	}

	void ScriptManager::showContextMenu(const QPoint& p)
	{
		context_menu->popup(view->mapToGlobal(p));
	}
	
	void ScriptManager::dataChanged(const QModelIndex & from,const QModelIndex & to)
	{
		Q_UNUSED(from);
		Q_UNUSED(to);
		updateActions(selectedScripts());
	}
}
