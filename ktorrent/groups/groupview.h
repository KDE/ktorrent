/*
    SPDX-FileCopyrightText: 2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTGROUPVIEW_H
#define KTGROUPVIEW_H

#include <KSharedConfig>
#include <QTreeWidget>

#include <groups/grouptreemodel.h>

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
class TorrentGroup;
class View;

/**
    @author Joris Guisson <joris.guisson@gmail.com>
*/
class GroupView : public QTreeView
{
    Q_OBJECT
public:
    GroupView(GroupManager *gman, View *view, Core *core, GUI *gui, QWidget *parent);
    ~GroupView() override;

    /// Save the status of the group view
    void saveState(KSharedConfigPtr cfg);

    /// Load status from config
    void loadState(KSharedConfigPtr cfg);

    /// Get a list of all expanded groups in the tree
    QStringList expandedGroupPaths() const;

    /// Appends the group given by index if it is expanded, and recursively adds any children of index
    void expandedGroupPaths(QStringList &groups, const QModelIndex &index) const;

    /// Expand the list of groups given by their paths
    void expandGroups(const QStringList &groupPaths);

    /// Create a new group
    Group *addNewGroup();

    /// Setup all the actions of the GroupView
    void setupActions(KActionCollection *col);

public Q_SLOTS:
    /// Update the group count
    void updateGroupCount();

private Q_SLOTS:
    void onItemClicked(const QModelIndex &index);
    void showContextMenu(const QPoint &p);
    void addGroup();
    void removeGroup();
    void editGroupName();
    void editGroupPolicy();
    void openInNewTab();

Q_SIGNALS:
    void currentGroupChanged(kt::Group *g);
    void openTab(Group *g);
    void addTorrentSelectionToGroup(TorrentGroup *g);

private:
    void keyPressEvent(QKeyEvent *event) override;

private:
    GUI *gui;
    Core *core;
    View *view;
    GroupManager *gman;
    GroupTreeModel *model;

    QAction *open_in_new_tab;
    QAction *new_group;
    QAction *edit_group;
    QAction *remove_group;
    QAction *edit_group_policy;

    friend class GroupViewItem;
};

}

#endif
