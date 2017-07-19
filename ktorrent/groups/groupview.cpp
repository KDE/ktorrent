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

#include "groupview.h"

#include <QAction>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QInputDialog>
#include <QMenu>
#include <QTreeWidgetItemIterator>

#include <KActionCollection>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>

#include <util/log.h>
#include <interfaces/torrentinterface.h>
#include <interfaces/torrentactivityinterface.h>
#include <groups/group.h>
#include <groups/groupmanager.h>
#include <groups/torrentgroup.h>
#include "view/view.h"
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

        connect(this, &GroupView::clicked, this, &GroupView::onItemClicked);
        connect(this, &GroupView::customContextMenuRequested, this, &GroupView::showContextMenu);
        connect(this, &GroupView::currentGroupChanged, view, &View::onCurrentGroupChanged);
        connect(gman, &GroupManager::customGroupChanged, this, &GroupView::updateGroupCount);

        setAcceptDrops(true);
        setDropIndicatorShown(true);
        setDragDropMode(QAbstractItemView::DropOnly);
    }


    GroupView::~GroupView()
    {
    }

    void GroupView::setupActions(KActionCollection* col)
    {
        open_in_new_tab = new QAction(QIcon::fromTheme(QStringLiteral("list-add")), i18n("Open In New Tab"), this);
        connect(open_in_new_tab, &QAction::triggered, this, &GroupView::openInNewTab);
        col->addAction(QStringLiteral("open_in_new_tab"), open_in_new_tab);

        new_group = new QAction(QIcon::fromTheme(QStringLiteral("document-new")), i18n("New Group"), this);
        connect(new_group, &QAction::triggered, this, &GroupView::addGroup);
        col->addAction(QStringLiteral("new_group"), new_group);

        edit_group = new QAction(QIcon::fromTheme(QStringLiteral("insert-text")), i18n("Edit Name"), this);
        connect(edit_group, &QAction::triggered, this, &GroupView::editGroupName);
        col->addAction(QStringLiteral("edit_group_name"), edit_group);

        remove_group = new QAction(QIcon::fromTheme(QStringLiteral("edit-delete")), i18n("Remove Group"), this);
        connect(remove_group, &QAction::triggered, this, &GroupView::removeGroup);
        col->addAction(QStringLiteral("remove_group"), remove_group);

        edit_group_policy = new QAction(QIcon::fromTheme(QStringLiteral("preferences-other")), i18n("Group Policy"), this);
        connect(edit_group_policy, &QAction::triggered, this, &GroupView::editGroupPolicy);
        col->addAction(QStringLiteral("edit_group_policy"), edit_group_policy);
    }

    void GroupView::addGroup()
    {
        addNewGroup();
    }

    Group* GroupView::addNewGroup()
    {
        bool ok = false;
        QString name = QInputDialog::getText(this, QString(), i18n("Please enter the group name."), QLineEdit::Normal, QString(), &ok);

        if (name.isEmpty() || name.length() == 0 || !ok)
            return nullptr;

        if (gman->find(name))
        {
            KMessageBox::error(this, i18n("The group %1 already exists.", name));
            return nullptr;
        }

        Group* g = gman->newGroup(name);
        gman->saveGroups();
        return g;
    }

    void GroupView::removeGroup()
    {
        Group* g = model->groupForIndex(selectionModel()->currentIndex());
        if (g)
        {
            gman->removeGroup(g);
            gman->saveGroups();
        }
    }

    void GroupView::editGroupName()
    {
        edit(selectionModel()->currentIndex());
    }

    void GroupView::showContextMenu(const QPoint& p)
    {
        Group* g = model->groupForIndex(selectionModel()->currentIndex());

        bool enable = g && gman->canRemove(g);
        edit_group->setEnabled(enable);
        remove_group->setEnabled(enable);
        edit_group_policy->setEnabled(enable);

        open_in_new_tab->setEnabled(g != 0);

        QMenu* menu = gui->getTorrentActivity()->part()->menu(QStringLiteral("GroupsMenu"));
        if (menu)
            menu->popup(viewport()->mapToGlobal(p));
    }

    void GroupView::onItemClicked(const QModelIndex& index)
    {
        Group* g = model->groupForIndex(index);
        if (g)
            currentGroupChanged(g);
    }

    void GroupView::editGroupPolicy()
    {
        Group* g = model->groupForIndex(selectionModel()->currentIndex());
        if (g)
        {
            GroupPolicyDlg dlg(g, this);
            if (dlg.exec() == QDialog::Accepted)
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
        default_expanded << QStringLiteral("/all")
                         << QStringLiteral("/all/downloads")
                         << QStringLiteral("/all/uploads")
                         << QStringLiteral("/all/active")
                         << QStringLiteral("/all/passive")
                         << QStringLiteral("/all/custom");
        QStringList slist = g.readEntry("expanded", default_expanded);
        model->expandGroups(this, slist);
        setVisible(g.readEntry("visible", true));
        expand(model->index(0, 0));
    }

    void GroupView::keyPressEvent(QKeyEvent* event)
    {
        if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
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
        if (g)
            openTab(g);
    }
}

