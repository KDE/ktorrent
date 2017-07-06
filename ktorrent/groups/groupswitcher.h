/***************************************************************************
 *   Copyright (C) 2012 by                                                 *
 *   Joris Guisson <joris.guisson@gmail.com>                               *
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

#ifndef KT_GROUPSWITCHER_H
#define KT_GROUPSWITCHER_H

#include <QActionGroup>

#include <KSharedConfig>
#include <KToolBar>

#include <groups/groupmanager.h>

class QToolButton;

namespace kt
{
    class View;
    class QueueManager;

    /**
     * toolbar to switch between groups
     */
    class GroupSwitcher : public QWidget
    {
        Q_OBJECT
    public:
        GroupSwitcher(View* view, GroupManager* gman, QWidget* parent);
        ~GroupSwitcher();

        /**
         * Load state of widget from config
         * @param cfg The config
         **/
        void loadState(KSharedConfig::Ptr cfg);

        /**
         * Save state of widget to config
         * @param cfg The config
         **/
        void saveState(KSharedConfig::Ptr cfg);

    public slots:
        /**
         * Add a tab
         * @param group The group of the tab
         **/
        void addTab(Group* group);

        /**
         * Add a new tab showing the all group.
         **/
        void newTab();

        /**
         * Close the current tab
         **/
        void closeTab();

        /**
         * An action was activated
         * @param action The action
         **/
        void onActivated(QAction* action);

        /**
         * The current group has changed
         * @param group The new group
         **/
        void currentGroupChanged(kt::Group* group);

        /**
         * Update the group count
         */
        void updateGroupCount();

        /**
         * A group has been removed
         * @param group The group
         **/
        void groupRemoved(Group* group);

        /**
         * Edit the group policy
         **/
        void editGroupPolicy();

    private:
        struct Tab
        {
            Group* group;
            QAction* action;
            QByteArray view_settings;

            Tab(Group* group, QAction* action) : group(group), action(action)
            {}
        };

        typedef QList<Tab> TabList;

        TabList::iterator closeTab(TabList::iterator i);

    private:
        QToolButton* new_tab;
        QToolButton* close_tab;
        QToolButton* edit_group_policy;
        KToolBar* tool_bar;
        QActionGroup* action_group;
        GroupManager* gman;
        View* view;
        TabList tabs;
        int current_tab;
    };

}

#endif // KT_GROUPSWITCHER_H
