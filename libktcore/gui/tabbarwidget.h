/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TABBARWIDGET_H
#define TABBARWIDGET_H

#include <QActionGroup>
#include <QList>
#include <QSplitter>
#include <QStackedWidget>
#include <QToolBar>

#include <KSharedConfig>

#include "ktcore_export.h"

namespace kt
{
class ActionGroup;

class KTCORE_EXPORT TabBarWidget : public QWidget
{
    Q_OBJECT
public:
    TabBarWidget(QSplitter *splitter, QWidget *parent);
    ~TabBarWidget() override;

    /// Add a tab to the TabBarWidget
    void addTab(QWidget *w, const QString &text, const QString &icon, const QString &tooltip);

    /// Remove a tab from the TabBarWidget
    void removeTab(QWidget *w);

    /// Save current status of sidebar, called at exit
    void saveState(KSharedConfigPtr cfg, const QString &group);

    /// Restore status from config, called at startup
    void loadState(KSharedConfigPtr cfg, const QString &group);

    /// Change the text of a tab
    void changeTabText(QWidget *w, const QString &text);

    /// Change the icon of a tab
    void changeTabIcon(QWidget *w, const QString &icon);

private Q_SLOTS:
    void onActionTriggered(QAction *act);
    void toolButtonStyleChanged(Qt::ToolButtonStyle style);
    void setToolButtonStyle();

private:
    void shrink();
    void unshrink();

private:
    QToolBar *tab_bar;
    ActionGroup *action_group;
    QStackedWidget *widget_stack;
    bool shrunken;
    QMap<QWidget *, QAction *> widget_to_action;
};

class ActionGroup : public QObject
{
    Q_OBJECT
public:
    ActionGroup(QObject *parent = 0);
    ~ActionGroup() override;

    void addAction(QAction *act);
    void removeAction(QAction *act);

private Q_SLOTS:
    void toggled(bool on);

Q_SIGNALS:
    void actionTriggered(QAction *a);

private:
    QList<QAction *> actions;
};
}

#endif // TABBARWIDGET_H
