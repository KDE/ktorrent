/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson                                   *
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
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QTreeWidgetItemIterator>
#include <klocale.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kmenu.h>
#include <kaction.h>
#include <kmessagebox.h>
#include <kinputdialog.h>
#include <kactioncollection.h>
#include <kconfiggroup.h>
#include <util/log.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/torrentactivityinterface.h>
#include <groups/group.h>
#include <groups/groupmanager.h>
#include <groups/torrentgroup.h>
#include "view/view.h"
#include "groupview.h"
#include "grouppolicydlg.h"
#include "gui.h"
#include "core.h"


using namespace bt;

namespace kt
{

	GroupView::GroupView(GroupManager* gman, View* view, Core* core, GUI* gui, QWidget* parent)
		: QTreeView(parent),
		  gui(gui),
		  core(core),
		  view(view),
		  gman(gman),
		  model(new GroupViewModel(gman, view, parent))
	{
		setRootIsDecorated(false);
		setContextMenuPolicy(Qt::CustomContextMenu);
		setModel(model);
		header()->hide();

		connect(this, SIGNAL(clicked(QModelIndex)), this, SLOT(onItemClicked(QModelIndex)));
		connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(showContextMenu(const QPoint&)));
		connect(this, SIGNAL(currentGroupChanged(kt::Group*)), view, SLOT(onCurrentGroupChanged(kt::Group*)));
		connect(gman, SIGNAL(customGroupChanged()), this, SLOT(updateGroupCount()));

		setAcceptDrops(true);
		setDropIndicatorShown(true);
		setDragDropMode(QAbstractItemView::DropOnly);
	}


	GroupView::~GroupView()
	{
	}

	void GroupView::setupActions(KActionCollection* col)
	{
		open_in_new_tab = new KAction(KIcon("list-add"), i18n("Open In New Tab"), this);
		connect(open_in_new_tab, SIGNAL(triggered()), this, SLOT(openInNewTab()));
		col->addAction("open_in_new_tab", open_in_new_tab);
		
		new_group = new KAction(KIcon("document-new"), i18n("New Group"), this);
		connect(new_group, SIGNAL(triggered()), this, SLOT(addGroup()));
		col->addAction("new_group", new_group);

		edit_group = new KAction(KIcon("insert-text"), i18n("Edit Name"), this);
		connect(edit_group, SIGNAL(triggered()), this, SLOT(editGroupName()));
		col->addAction("edit_group_name", edit_group);

		remove_group = new KAction(KIcon("edit-delete"), i18n("Remove Group"), this);
		connect(remove_group, SIGNAL(triggered()), this, SLOT(removeGroup()));
		col->addAction("remove_group", remove_group);

		edit_group_policy = new KAction(KIcon("preferences-other"), i18n("Group Policy"), this);
		connect(edit_group_policy, SIGNAL(triggered()), this, SLOT(editGroupPolicy()));
		col->addAction("edit_group_policy", edit_group_policy);
	}

	void GroupView::addGroup()
	{
		addNewGroup();
	}

	Group* GroupView::addNewGroup()
	{
		bool ok = false;
		QString name = KInputDialog::getText(QString(), i18n("Please enter the group name."), QString(), &ok, this);

		if(name.isNull() || name.length() == 0 || !ok)
			return 0;

		if(gman->find(name))
		{
			KMessageBox::error(this, i18n("The group %1 already exists.", name));
			return 0;
		}

		Group* g = gman->newGroup(name);
		gman->saveGroups();
		return g;
	}

	void GroupView::removeGroup()
	{
		Group* g = model->groupForIndex(selectionModel()->currentIndex());
		if(g)
		{
			gman->removeGroup(g);
			gman->saveGroups();
		}
	}

	void GroupView::editGroupName()
	{
		edit(selectionModel()->currentIndex());
	}

	void GroupView::showContextMenu(const QPoint & p)
	{
		Group* g = model->groupForIndex(selectionModel()->currentIndex());
		
		if(!g || !gman->canRemove(g))
		{
			edit_group->setEnabled(false);
			remove_group->setEnabled(false);
			edit_group_policy->setEnabled(false);
		}
		else
		{
			edit_group->setEnabled(true);
			remove_group->setEnabled(true);
			edit_group_policy->setEnabled(true);
		}
		
		open_in_new_tab->setEnabled(g != 0);

		KMenu* menu = gui->getTorrentActivity()->part()->menu("GroupsMenu");
		if(menu)
			menu->popup(viewport()->mapToGlobal(p));
	}
	
	void GroupView::onItemClicked(const QModelIndex& index)
	{
		Group* g = model->groupForIndex(index);
		if(g)
			currentGroupChanged(g);
	}

	void GroupView::editGroupPolicy()
	{
		Group* g = model->groupForIndex(selectionModel()->currentIndex());
		if(g)
		{
			GroupPolicyDlg dlg(g, this);
			if(dlg.exec() == QDialog::Accepted)
				gman->saveGroups();
		}
	}

	void GroupView::saveState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("GroupView");
		g.writeEntry("expanded", model->expandedGroups(this));
		g.writeEntry("visible", isVisible());
	}


	void GroupView::loadState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("GroupView");
		QStringList default_expanded;
		default_expanded << "/all" << "/all/downloads" << "/all/uploads" << "/all/active" << "/all/passive" << "/all/custom";
		QStringList slist = g.readEntry("expanded", default_expanded);
		model->expandGroups(this, slist);
		setVisible(g.readEntry("visible", true));
		expand(model->index(0, 0));
	}

	void GroupView::keyPressEvent(QKeyEvent* event)
	{
		if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
			onItemClicked(selectionModel()->currentIndex());
		else
			QTreeView::keyPressEvent(event);
	}
	
	void GroupView::updateGroupCount()
	{
		model->updateGroupCount(model->index(0, 0));
	}
	
	void GroupView::openInNewTab()
	{
		Group* g = model->groupForIndex(selectionModel()->currentIndex());
		if(g)
			openTab(g);
	}
}

#include "groupview.moc"
