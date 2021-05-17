/*
    SPDX-FileCopyrightText: 2012 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <KConfigGroup>

#include <QAction>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIcon>
#include <QToolButton>

#include "grouppolicydlg.h"
#include "groupswitcher.h"
#include <torrent/queuemanager.h>
#include <util/log.h>
#include <view/view.h>

using namespace bt;

namespace kt
{
GroupSwitcher::GroupSwitcher(View *view, GroupManager *gman, QWidget *parent)
    : QWidget(parent)
    , new_tab(new QToolButton(this))
    , close_tab(new QToolButton(this))
    , edit_group_policy(new QToolButton(this))
    , tool_bar(new KToolBar(this))
    , action_group(new QActionGroup(this))
    , gman(gman)
    , view(view)
    , current_tab(0)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(new_tab);
    layout->addWidget(edit_group_policy);
    layout->addWidget(tool_bar);
    layout->addWidget(close_tab);
    layout->setMargin(0);

    new_tab->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    new_tab->setToolButtonStyle(Qt::ToolButtonIconOnly);
    new_tab->setToolTip(i18n("Open a new tab"));
    connect(new_tab, &QToolButton::clicked, this, &GroupSwitcher::newTab);

    close_tab->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
    close_tab->setToolButtonStyle(Qt::ToolButtonIconOnly);
    close_tab->setToolTip(i18n("Close the current tab"));
    connect(close_tab, &QToolButton::clicked, this, qOverload<>(&GroupSwitcher::closeTab));

    edit_group_policy->setIcon(QIcon::fromTheme(QStringLiteral("preferences-other")));
    edit_group_policy->setToolButtonStyle(Qt::ToolButtonIconOnly);
    edit_group_policy->setToolTip(i18n("Edit Group Policy"));
    connect(edit_group_policy, &QToolButton::clicked, this, &GroupSwitcher::editGroupPolicy);

    tool_bar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    action_group->setExclusive(true);
    connect(action_group, &QActionGroup::triggered, this, &GroupSwitcher::onActivated);

    connect(gman, &GroupManager::groupRemoved, this, &GroupSwitcher::groupRemoved);
}

GroupSwitcher::~GroupSwitcher()
{
}

void GroupSwitcher::loadState(KSharedConfig::Ptr cfg)
{
    KConfigGroup g = cfg->group("GroupSwitcher");

    QStringList default_groups;
    default_groups << QStringLiteral("/all") << QStringLiteral("/all/downloads") << QStringLiteral("/all/uploads");

    const QStringList groups = g.readEntry("groups", default_groups);
    for (const QString &group : groups) {
        addTab(gman->findByPath(group));
    }

    if (tabs.isEmpty()) {
        for (const QString &group : qAsConst(default_groups))
            addTab(gman->findByPath(group));
    }

    int idx = 0;
    for (Tab &tab : tabs) {
        tab.view_settings = g.readEntry(QStringLiteral("tab%1_settings").arg(idx++), view->defaultState());
    }

    updateGroupCount();
    connect(gman, &GroupManager::customGroupChanged, this, &GroupSwitcher::updateGroupCount);

    current_tab = g.readEntry("current_tab", 0);
    if (current_tab >= 0 && current_tab < tabs.count()) {
        Tab &ct = tabs[current_tab];
        ct.action->setChecked(true);
        view->setGroup(ct.group);
        view->restoreState(ct.view_settings);
    } else {
        tabs.first().action->setChecked(true);
        view->setGroup(tabs.first().group);
        view->restoreState(tabs.first().view_settings);
        current_tab = 0;
    }

    edit_group_policy->setEnabled(!tabs.at(current_tab).group->isStandardGroup());
}

void GroupSwitcher::saveState(KSharedConfig::Ptr cfg)
{
    KConfigGroup g = cfg->group("GroupSwitcher");
    QStringList groups;
    int idx = 0;
    for (Tab &tab : tabs) {
        groups << tab.group->groupPath();
        if (idx == current_tab)
            tab.view_settings = view->header()->saveState();
        g.writeEntry(QStringLiteral("tab%1_settings").arg(idx++), tab.view_settings);
    }

    g.writeEntry("groups", groups);
    g.writeEntry("current_tab", current_tab);
}

void GroupSwitcher::addTab(Group *group)
{
    if (!group)
        return;

    QString name = group->groupName() + QStringLiteral(" %1/%2").arg(group->runningTorrents()).arg(group->totalTorrents());
    QAction *action = tool_bar->addAction(group->groupIcon(), name);
    action->setCheckable(true);
    action_group->addAction(action);
    tabs.append(Tab(group, action));

    action->toggle();
    view->setGroup(group);
    view->restoreState(view->defaultState());
    tabs.last().view_settings = view->header()->saveState();
    current_tab = tabs.count() - 1;

    close_tab->setEnabled(tabs.count() > 1);
}

void GroupSwitcher::newTab()
{
    addTab(gman->allGroup());
}

GroupSwitcher::TabList::iterator GroupSwitcher::closeTab(TabList::iterator i)
{
    action_group->removeAction(i->action);
    tool_bar->removeAction(i->action);
    i->action->deleteLater();
    TabList::iterator ret = tabs.erase(i);
    tabs.first().action->toggle();
    view->setGroup(tabs.first().group);
    view->restoreState(tabs.first().view_settings);
    current_tab = 0;
    return ret;
}

void GroupSwitcher::closeTab()
{
    if (tabs.size() <= 1) // Need at least one tab visible
        return;

    TabList::iterator i = tabs.begin();
    while (i != tabs.end()) {
        if (i->action->isChecked()) {
            closeTab(i);
            break;
        }
        i++;
    }

    close_tab->setEnabled(tabs.count() > 1);
}

void GroupSwitcher::onActivated(QAction *action)
{
    tabs[current_tab].view_settings = view->header()->saveState();

    int idx = 0;
    for (const Tab &tab : qAsConst(tabs)) {
        if (tab.action == action) {
            view->setGroup(tab.group);
            view->restoreState(tab.view_settings);
            current_tab = idx;
            edit_group_policy->setEnabled(!tab.group->isStandardGroup());
            break;
        }
        idx++;
    }
}

void GroupSwitcher::currentGroupChanged(Group *group)
{
    for (Tab &tab : tabs) {
        if (tab.action->isChecked()) {
            tab.group = group;
            QString name = group->groupName() + QStringLiteral(" %1/%2").arg(group->runningTorrents()).arg(group->totalTorrents());
            tab.action->setText(name);
            tab.action->setIcon(group->groupIcon());
            edit_group_policy->setEnabled(!group->isStandardGroup());
            break;
        }
    }
}

void GroupSwitcher::updateGroupCount()
{
    for (Tab &tab : tabs)
        tab.action->setText(tab.group->groupName() + QStringLiteral(" %1/%2").arg(tab.group->runningTorrents()).arg(tab.group->totalTorrents()));
}

void GroupSwitcher::groupRemoved(Group *group)
{
    for (TabList::iterator i = tabs.begin(); i != tabs.end();) {
        if (i->group == group) {
            if (tabs.size() > 1) {
                i = closeTab(i);
            } else {
                i->group = gman->allGroup();
                i->action->setIcon(i->group->groupIcon());
                i->action->setText(i->group->groupName() + QStringLiteral(" %1/%2").arg(i->group->runningTorrents()).arg(i->group->totalTorrents()));
                i++;
            }
        } else
            i++;
    }
}

void GroupSwitcher::editGroupPolicy()
{
    Group *g = tabs[current_tab].group;
    if (g) {
        GroupPolicyDlg dlg(g, this);
        if (dlg.exec() == QDialog::Accepted)
            gman->saveGroups();
    }
}

}
