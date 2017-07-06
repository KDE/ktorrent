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

#ifndef KTGROUPVIEW_H
#define KTGROUPVIEW_H

#include <QTreeWidget>
#include <KSharedConfig>

#include "groupviewmodel.h"

class QAction;
class KActionCollection;

namespace kt
{
    class GUI;
    class Core;
    class View;
    class Group;
    class GroupView;
    class GroupManager;
    class View;


    /**
        @author Joris Guisson <joris.guisson@gmail.com>
    */
    class GroupView : public QTreeView
    {
        Q_OBJECT
    public:
        GroupView(GroupManager* gman, View* view, Core* core, GUI* gui, QWidget* parent);
        ~GroupView();

        /// Save the status of the group view
        void saveState(KSharedConfigPtr cfg);

        /// Load status from config
        void loadState(KSharedConfigPtr cfg);

        /// Create a new group
        Group* addNewGroup();

        /// Setup all the actions of the GroupView
        void setupActions(KActionCollection* col);

    public slots:
        /// Update the group count
        void updateGroupCount();

    private slots:
        void onItemClicked(const QModelIndex& index);
        void showContextMenu(const QPoint& p);
        void addGroup();
        void removeGroup();
        void editGroupName();
        void editGroupPolicy();
        void openInNewTab();

    signals:
        void currentGroupChanged(kt::Group* g);
        void openTab(Group* g);

    private:
        void keyPressEvent(QKeyEvent* event) override;

    private:
        GUI* gui;
        Core* core;
        View* view;
        GroupManager* gman;
        GroupViewModel* model;

        QAction * open_in_new_tab;
        QAction * new_group;
        QAction * edit_group;
        QAction * remove_group;
        QAction * edit_group_policy;

        friend class GroupViewItem;
    };

}

#endif
