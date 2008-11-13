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
#include <groups/group.h>
#include <groups/groupmanager.h>
#include <groups/torrentgroup.h>
#include "viewmanager.h"
#include "view.h"
#include "groupview.h"
#include "grouppolicydlg.h"
#include "gui.h"


using namespace bt;

namespace kt
{

	GroupViewItem::GroupViewItem(GroupView* parent,Group* g,const QString & name) : QTreeWidgetItem(parent),g(g),path_name(name)
	{
		if (g)
		{
			setText(0,g->groupName());
			setIcon(0,g->groupIcon());
		}
		else
		{
			setText(0,path_name);
			setIcon(0,KIcon("folder"));
		}
	}
	
	GroupViewItem::GroupViewItem(QTreeWidgetItem* parent,Group* g,const QString & name) : QTreeWidgetItem(parent),g(g),path_name(name)
	{
		if (g)
		{
			setText(0,g->groupName());
			setIcon(0,g->groupIcon());
		}
		else
		{
			setText(0,path_name);
			setIcon(0,KIcon("folder"));
		}
	}
	
	GroupViewItem::~GroupViewItem()
	{
	}
	
	void GroupViewItem::setGroup(Group* g)
	{
		this->g = g;
		setText(0,g->groupName());
		setIcon(0,g->groupIcon());
	}
	
	/*
	int GroupViewItem::compare(QListViewItem* i,int ,bool ) const
	{
		if (text(1).isNull() && i->text(1).isNull())
			return QString::compare(text(0),i->text(0));
		else
			return QString::compare(text(1),i->text(1));
	}
	*/

	GroupView::GroupView(GroupManager* gman,ViewManager* view,GUI* gui)
	: QTreeWidget(gui),gui(gui),view(view),custom_root(0),gman(gman)
	{
		setColumnCount(1);
		setContextMenuPolicy(Qt::CustomContextMenu);
		headerItem()->setHidden(true);
	
		current = gman->allGroup();
		
		connect(this,SIGNAL(itemClicked(QTreeWidgetItem*,int)),this,SLOT(onItemActivated(QTreeWidgetItem*,int)));
		connect(this,SIGNAL(customContextMenuRequested(const QPoint & ) ),this,SLOT(showContextMenu( const QPoint& )));
		connect(this,SIGNAL(itemChanged(QTreeWidgetItem*,int)),this,SLOT(onItemChanged(QTreeWidgetItem*,int)));
		connect(this,SIGNAL(currentGroupChanged(kt::Group*)),view,SLOT(onCurrentGroupChanged(kt::Group*)));
		connect(this,SIGNAL(groupRenamed(kt::Group*)),view,SLOT(onGroupRenamed(kt::Group*)));
		connect(gman,SIGNAL(customGroupRemoved(Group*)),this,SLOT(customGroupRemoved(Group*)));
		connect(gman,SIGNAL(customGroupAdded(Group*)),this,SLOT(customGroupAdded(Group*)));
		connect(gman,SIGNAL(defaultGroupRemoved(Group*)),this,SLOT(defaultGroupRemoved(Group*)));
		connect(gman,SIGNAL(defaultGroupAdded(Group*)),this,SLOT(defaultGroupAdded(Group*)));

		current_item = 0;
		menu = 0;
		setupActions(gui->actionCollection());
		
		
		for (GroupManager::DefGroupItr i = gman->beginDefaults();i != gman->endDefaults();i++)
		{
			Group* g = *i;
			add(0,g->groupPath(),g);
		}
		
		custom_root = add(0,"/all/custom",0);
		custom_root->setText(0,i18n("Custom Groups"));
		custom_root->setIcon(0,SmallIcon("folder"));
		custom_root->setExpanded(true);
		
		for (GroupManager::iterator i = gman->begin();i != gman->end();i++)
		{
			GroupViewItem* gvi = addGroup(i->second,custom_root,i->second->groupName());
			gvi->setFlags(gvi->flags() | Qt::ItemIsEditable | Qt::ItemIsDropEnabled);
		}

		setAcceptDrops(true);
		setDropIndicatorShown(true);
		setDragDropMode(QAbstractItemView::DropOnly);
	}


	GroupView::~GroupView()
	{	
	}
	
	void GroupView::setupActions(KActionCollection* col)
	{
		new_group = new KAction(KIcon("document-new"),i18n("New Group"),this);
		connect(new_group,SIGNAL(triggered()),this,SLOT(addGroup()));
		col->addAction("new_group",new_group);

		edit_group = new KAction(KIcon("insert-text"),i18n("Edit Name"),this);
		connect(edit_group,SIGNAL(triggered()),this,SLOT(editGroupName()));
		col->addAction("edit_group_name",edit_group);
		
		remove_group = new KAction(KIcon("edit-delete"),i18n("Remove Group"),this);
		connect(remove_group,SIGNAL(triggered()),this,SLOT(removeGroup()));
		col->addAction("remove_group",remove_group);
		
		open_in_new_tab = new KAction(KIcon("tab-new"),i18n("Open Tab"),this);
		connect(open_in_new_tab,SIGNAL(triggered()),this,SLOT(openView()));
		col->addAction("open_tab",open_in_new_tab);
		
		edit_group_policy = new KAction(KIcon("preferences-other"),i18n("Group Policy"),this);
		connect(edit_group_policy,SIGNAL(triggered()),this,SLOT(editGroupPolicy()));
		col->addAction("edit_group_policy",edit_group_policy);
	}
	
	GroupViewItem* GroupView::add(QTreeWidgetItem* parent,const QString & path,Group* g)
	{
		// if path looks like /foo we are at a leaf of the tree
		if (path.count('/') == 1)
		{
			QString name = path.mid(1);
			if (parent)
			{
				// see if we can find a GroupViewItem with the same name and which is a child of parent
				for (int i = 0;i < parent->childCount();i++)
				{
					GroupViewItem* gvi = (GroupViewItem*)parent->child(i);
					if (gvi->name() == name)
					{
						// there is one so just fill in the group
						gvi->setGroup(g);
						return gvi;
					}
				}
			}
			else
			{
				for (int i = 0;i < topLevelItemCount();i++)
				{
					GroupViewItem* gvi = (GroupViewItem*)topLevelItem(i);
					if (gvi->name() == name)
					{
						// there is one so just fill in the group
						gvi->setGroup(g);
						return gvi;
					}
				}
			}
			
			// no existing one found, so create a new one
			return addGroup(g,parent,name);
		}
		else
		{
			QString p = path.mid(1); // get rid of first slash
			int slash_pos = p.indexOf('/'); // find position of slash
			QString name = p.mid(0,slash_pos); // get the name
			p = p.mid(slash_pos); // p now becomes next part of path
		
			// see if we can find a GroupViewItem with the same name and which is a child of parent
			if (parent)
			{
				for (int i = 0;i < parent->childCount();i++)
				{
					GroupViewItem* gvi = (GroupViewItem*)parent->child(i);
					if (gvi->name() == name)
					{
						// there is one, go on recusively
						return add(gvi,p,g);
					}
				}
			}
			else
			{
				for (int i = 0;i < topLevelItemCount();i++)
				{
					GroupViewItem* gvi = (GroupViewItem*)topLevelItem(i);
					if (gvi->name() == name)
					{
						// there is one, go on recusively
						return add(gvi,p,g);
					}
				}
			}
			
			// create a new empty GroupViewItem and go on recursively
			GroupViewItem* gvi = addGroup(0,parent,name);
			return add(gvi,p,g);
		}
	}
	
	void GroupView::remove(QTreeWidgetItem* parent,const QString & path,Group* g)
	{
		// if path looks like /foo we are at a leaf of the tree
		if (path.count('/') == 1)
		{
			QString name = path.mid(1);
			if (parent)
			{
				for (int i = 0;i < parent->childCount();i++)
				{
					GroupViewItem* gvi = (GroupViewItem*)parent->child(i);
					if (gvi->group() == g)
					{
						// we have found it
						delete gvi;
						return;
					}
				}
			}
			else
			{
				for (int i = 0;i < topLevelItemCount();i++)
				{
					GroupViewItem* gvi = (GroupViewItem*)topLevelItem(i);
					if (gvi->group() == g)
					{
						// we have found it
						delete gvi;
						return;
					}
				}
			}
		}
		else
		{
			QString p = path.mid(1); // get rid of first slash
			int slash_pos = p.indexOf('/'); // find position of slash
			QString name = p.mid(0,slash_pos); // get the name
			p = p.mid(slash_pos); // p now becomes next part of path
			
			// see if we can find a GroupViewItem with the same name and which is a child of parent
			if (parent)
			{
				for (int i = 0;i < parent->childCount();i++)
				{
					GroupViewItem* gvi = (GroupViewItem*)parent->child(i);
					if (gvi->name() == name)
					{
						// there is one, go on recusively
						remove(gvi,p,g);
						// remove empty maps, if they are not the custom map
						if (gvi->childCount() == 0 && gvi != custom_root)
							delete gvi;
						
						return;
					}
				}
			}
			else
			{
				for (int i = 0;i < topLevelItemCount();i++)
				{
					GroupViewItem* gvi = (GroupViewItem*)topLevelItem(i);
					if (gvi->name() == name)
					{
						// there is one, go on recusively
						remove(gvi,p,g);
						return;
					}
				}
			}
		}
	}
	
	
	void GroupView::addGroup()
	{
		addNewGroup();
	}
	
	Group* GroupView::addNewGroup()
	{
		bool ok = false;
		QString name = KInputDialog::getText(QString(),i18n("Please enter the group name."),QString(),&ok,this);
		
		if (name.isNull() || name.length() == 0 || !ok)
			return 0;
		
		if (gman->find(name))
		{
			KMessageBox::error(this,i18n("The group %1 already exists.",name));
			return 0;
		}
		
		Group* g = gman->newGroup(name);
		gman->saveGroups();
		return g;
	}
	
	void GroupView::removeGroup()
	{
		if (!current_item)
			return;
		
		Group* g = current_item->group();
		if (!g)
			return;
		
		gman->removeGroup(g);		
		current_item = 0;
		gman->saveGroups();
	}
	
	void GroupView::editGroupName()
	{
		if (!current_item)
			return;
		
		editItem(current_item);
	}

	GroupViewItem* GroupView::addGroup(Group* g,QTreeWidgetItem* parent,const QString & name)
	{
		// Note: g can be 0
		GroupViewItem* li = 0;
		if (parent)
		{
			li = new GroupViewItem(parent,g,name);
			li->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		}
		else
		{
			li = new GroupViewItem(this,g,name);
			if (g)
				li->setText(1,g->groupName());
			li->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			addTopLevelItem(li);
		}
		
		if (custom_root && custom_root->childCount() == 1 && custom_root == parent)
			custom_root->setExpanded(true);
		
		return li;
	}
	
	void GroupView::showContextMenu(const QPoint & p)
	{
		current_item = dynamic_cast<GroupViewItem*>(itemAt(p));
		
		Group* g = 0;
		if (current_item)
			g = current_item->group();
		
		if (!g || !gman->canRemove(g))
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
		
		if (!menu)
			menu = (KMenu*)gui->container("GroupsMenu");
		
		if (menu)
			menu->popup(mapToGlobal(p));
	}
	
	void GroupView::onItemActivated(QTreeWidgetItem* item,int)
	{
		if (!item) return;
		
		GroupViewItem* li = dynamic_cast<GroupViewItem*>(item);
		if (!li)
			return;
		
		Group* g = li->group();
		if (g)
		{
			current = g;
			currentGroupChanged(g);
		}
	}

	void GroupView::onItemChanged(QTreeWidgetItem* item,int )
	{
		if (!item) 
			return;

		GroupViewItem* li = dynamic_cast<GroupViewItem*>(item);
		if (!li)
			return;
		
		Group* g = li->group();
		if (g)
		{
			QString name = item->text(0);
			if (name.isNull() || name.length() == 0)
			{
				item->setText(0,g->groupName());
				return;
			}
		
			if (g->groupName() == name)
				return;
		
			if (gman->find(name)) 
			{
				KMessageBox::error(this,i18n("The group %1 already exists.",name));
				item->setText(0,g->groupName());
			}
			else
			{
				gman->renameGroup(g->groupName(),name);
				li->setText(0,name);
				groupRenamed(g);
				gman->saveGroups();
			//	sort();
			}
		}
	}
	
	bool GroupView::dropMimeData(QTreeWidgetItem *parent, int index,const QMimeData *data,Qt::DropAction action)			  
	{
		Q_UNUSED(index);
		Q_UNUSED(data);
		Q_UNUSED(action);
		GroupViewItem* li = dynamic_cast<GroupViewItem*>(parent);
		if (!li)
			return false;
		
		TorrentGroup* g = dynamic_cast<TorrentGroup*>(li->group());
		if (!g)
			return false;
		
		QList<TorrentInterface*> sel;
		view->getSelection(sel);
		foreach (TorrentInterface* ti,sel)
		{
			g->add(ti);
		}
		gman->saveGroups();
		return true;
	}
	
	QStringList GroupView::mimeTypes() const
	{
		QStringList sl;
		sl << "application/x-ktorrent-drag-object";
		return sl;
	}
	
	Qt::DropActions GroupView::supportedDropActions () const
	{
		return Qt::CopyAction;
	}

	void GroupView::openView()
	{
		if (!current_item)
			return;
		
		Group* g = current_item->group();
		if (g)
			openNewTab(g);
	}
	
	void GroupView::editGroupPolicy()
	{
		Group* g = current_item->group();
		if (g)
		{
			GroupPolicyDlg dlg(g,this);
			if (dlg.exec() == QDialog::Accepted)
				gman->saveGroups();
		}
	}
	
	void GroupView::saveState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("GroupView");
		QTreeWidgetItemIterator it(this);
		while (*it) 
		{
			if ((*it)->childCount() > 0)
				g.writeEntry((*it)->text(0),(*it)->isExpanded());
				
			++it;
		}
	}


	void GroupView::loadState(KSharedConfigPtr cfg)
	{
		KConfigGroup g = cfg->group("GroupView");
		QTreeWidgetItemIterator it(this);
		while (*it) 
		{
			if ((*it)->childCount() > 0)
				(*it)->setExpanded(g.readEntry((*it)->text(0),true));
				
			++it;
		}
	}
	
	void GroupView::defaultGroupAdded(Group* g)
	{
		Out(SYS_GEN|LOG_DEBUG) << "GroupView::defaultGroupAdded " << g->groupName() << " " << g->groupPath() << endl;
		add(0,g->groupPath(),g);
	}
	
	
	
	void GroupView::defaultGroupRemoved(Group* g)
	{
		Out(SYS_GEN|LOG_DEBUG) << "GroupView::defaultGroupRemoved " << g->groupName() << " " << g->groupPath() << endl;
		remove(0,g->groupPath(),g);
	}

	void GroupView::customGroupAdded(Group* g)
	{
		add(0,g->groupPath(),g);
		view->onGroupAdded(g);
	}
	
	void GroupView::customGroupRemoved(Group* g)
	{
		if (g == current)
		{
			current = gman->allGroup();
			currentGroupChanged(current);
		}
		
		view->onGroupRemoved(g);
		remove(0,g->groupPath(),g);
	}
}

#include "groupview.moc"
