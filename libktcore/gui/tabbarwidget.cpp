/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "tabbarwidget.h"

#include <KConfigGroup>
#include <KToolBar>
#include <QAction>

#include <QIcon>
#include <QTimer>
#include <QVBoxLayout>

namespace kt
{
ActionGroup::ActionGroup(QObject *parent)
    : QObject(parent)
{
}

ActionGroup::~ActionGroup()
{
}

void ActionGroup::addAction(QAction *act)
{
    actions.append(act);
    connect(act, &QAction::toggled, this, &ActionGroup::toggled);
}

void ActionGroup::removeAction(QAction *act)
{
    actions.removeAll(act);
    disconnect(act, &QAction::toggled, this, &ActionGroup::toggled);
}

void ActionGroup::toggled(bool on)
{
    QAction *act = qobject_cast<QAction *>(sender());
    if (!act)
        return;

    for (QAction *a : qAsConst(actions)) {
        if (a != act)
            a->setChecked(false);
    }

    act->setChecked(on);
    Q_EMIT actionTriggered(act);
}

TabBarWidget::TabBarWidget(QSplitter *splitter, QWidget *parent)
    : QWidget(parent)
    , widget_stack(nullptr)
    , shrunken(false)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(0);
    tab_bar = new KToolBar(this);
    tab_bar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    connect(tab_bar, &QToolBar::toolButtonStyleChanged, this, &TabBarWidget::toolButtonStyleChanged);
    action_group = new ActionGroup(this);
    connect(action_group, &ActionGroup::actionTriggered, this, &TabBarWidget::onActionTriggered);
    widget_stack = new QStackedWidget(splitter);
    splitter->addWidget(widget_stack);
    layout->addWidget(tab_bar);

    QSizePolicy tsp = sizePolicy();
    QSizePolicy wsp = widget_stack->sizePolicy();

    tsp.setVerticalPolicy(QSizePolicy::Fixed);
    wsp.setVerticalPolicy(QSizePolicy::Expanding);

    widget_stack->setSizePolicy(wsp);
    setSizePolicy(tsp);
    shrink();
}

TabBarWidget::~TabBarWidget()
{
}

void TabBarWidget::addTab(QWidget *ti, const QString &text, const QString &icon, const QString &tooltip)
{
    QAction *act = tab_bar->addAction(QIcon::fromTheme(icon), text);
    act->setCheckable(true);
    act->setToolTip(tooltip);
    act->setChecked(widget_stack->count() == 0 && !shrunken);
    widget_stack->addWidget(ti);
    action_group->addAction(act);
    widget_to_action.insert(ti, act);
    show();
}

void TabBarWidget::removeTab(QWidget *ti)
{
    QMap<QWidget *, QAction *>::iterator itr = widget_to_action.find(ti);
    if (itr == widget_to_action.end())
        return;

    tab_bar->removeAction(itr.value());
    action_group->removeAction(itr.value());
    itr.value()->deleteLater();
    if (widget_stack->currentWidget() == ti) {
        ti->hide();
        widget_stack->removeWidget(ti);
        ti->setParent(nullptr);
    } else {
        widget_stack->removeWidget(ti);
        ti->setParent(nullptr);
    }

    if (widget_stack->count() == 0) {
        widget_stack->hide();
        hide();
    } else {
        QMap<QWidget *, QAction *>::iterator itr = widget_to_action.find(widget_stack->currentWidget());
        if (itr != widget_to_action.end()) {
            QAction *act = itr.value();
            act->setChecked(true);
        }
    }
}

void TabBarWidget::changeTabIcon(QWidget *ti, const QString &icon)
{
    QMap<QWidget *, QAction *>::iterator itr = widget_to_action.find(ti);
    if (itr == widget_to_action.end())
        return;

    itr.value()->setIcon(QIcon::fromTheme(icon));
}

void TabBarWidget::changeTabText(QWidget *ti, const QString &text)
{
    QMap<QWidget *, QAction *>::iterator itr = widget_to_action.find(ti);
    if (itr == widget_to_action.end())
        return;

    itr.value()->setText(text);
}

void TabBarWidget::shrink()
{
    widget_stack->hide();
    shrunken = true;
}

void TabBarWidget::unshrink()
{
    widget_stack->show();
    shrunken = false;
}

void TabBarWidget::onActionTriggered(QAction *act)
{
    QWidget *ti = nullptr;
    QMap<QWidget *, QAction *>::iterator i = widget_to_action.begin();
    while (i != widget_to_action.end() && !ti) {
        if (i.value() == act)
            ti = i.key();
        i++;
    }

    if (!ti)
        return;

    if (ti == widget_stack->currentWidget()) {
        // it is the current tab
        if (act->isChecked())
            unshrink();
        else
            shrink();
    } else {
        // change the current in stack
        widget_stack->setCurrentWidget(ti);
        if (shrunken)
            unshrink();
    }
}

void TabBarWidget::saveState(KSharedConfigPtr cfg, const QString &group)
{
    QWidget *current = widget_stack->currentWidget();

    KConfigGroup g = cfg->group(group);
    g.writeEntry("shrunken", shrunken);
    if (current)
        g.writeEntry("current_tab", widget_to_action[current]->text());
}

void TabBarWidget::loadState(KSharedConfigPtr cfg, const QString &group)
{
    KConfigGroup g = cfg->group(group);

    bool tmp = g.readEntry("shrunken", true);
    if (tmp != shrunken) {
        if (tmp)
            shrink();
        else
            unshrink();
    }

    QString ctab = g.readPathEntry("current_tab", QString());
    for (QMap<QWidget *, QAction *>::const_iterator i = widget_to_action.cbegin(); i != widget_to_action.cend(); i++) {
        if (i.value()->text() == ctab) {
            widget_stack->setCurrentWidget(i.key());
            i.value()->setChecked(!tmp);
            break;
        }
    }
}

void TabBarWidget::toolButtonStyleChanged(Qt::ToolButtonStyle style)
{
    if (style != Qt::ToolButtonTextBesideIcon)
        QTimer::singleShot(0, this, &TabBarWidget::setToolButtonStyle);
}

void TabBarWidget::setToolButtonStyle()
{
    tab_bar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
}

}
