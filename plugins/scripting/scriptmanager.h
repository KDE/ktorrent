/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KTSCRIPTMANAGER_H
#define KTSCRIPTMANAGER_H

#include <QListView>
#include <interfaces/activity.h>

class QAction;
class KActionCollection;

namespace Kross
{
class Action;
}

namespace kt
{
class Script;
class ScriptModel;
class ScriptDelegate;

/**
    Widget to display all scripts.
*/
class ScriptManager : public Activity
{
    Q_OBJECT
public:
    ScriptManager(ScriptModel *model, QWidget *parent);
    ~ScriptManager() override;

    /// Get all selected scripts
    QModelIndexList selectedScripts();

    /// Update all actions and make sure they are properly enabled or disabled
    void updateActions(const QModelIndexList &selected);

private Q_SLOTS:
    void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void showContextMenu(const QPoint &p);
    void dataChanged(const QModelIndex &f, const QModelIndex &to);
    void runScript();
    void stopScript();
    void editScript();
    void configureScript();
    void showProperties();

public:
    void showProperties(Script *script);

private:
    void setupActions();

Q_SIGNALS:
    void addScript();
    void removeScript();

private:
    ScriptModel *model;
    ScriptDelegate *delegate;
    QListView *view;

    QAction *add_script;
    QAction *remove_script;
    QAction *run_script;
    QAction *stop_script;
    QAction *edit_script;
    QAction *properties;
    QAction *configure_script;
};

}

#endif
