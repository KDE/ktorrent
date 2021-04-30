/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <KConfigGroup>
#include <QAction>
#include <QIcon>

#include "centralwidget.h"
#include <interfaces/activity.h>

namespace kt
{
CentralWidget::CentralWidget(QWidget *parent)
    : QStackedWidget(parent)
{
    activity_switching_group = new QActionGroup(this);
    connect(activity_switching_group, &QActionGroup::triggered, this, &CentralWidget::switchActivity);
}

CentralWidget::~CentralWidget()
{
}

void CentralWidget::loadState(KSharedConfigPtr cfg)
{
    KConfigGroup g = cfg->group("MainWindow");
    int idx = g.readEntry("current_activity", 0);
    Activity *act = (Activity *)widget(idx);
    if (act)
        setCurrentActivity(act);

    const QList<QAction *> actions = activity_switching_group->actions();
    for (QAction *a : actions) {
        if (a->data().value<QObject *>() == act || act == 0) {
            a->setChecked(true);
            break;
        }
    }

    for (QAction *a : actions) {
        a->setPriority((QAction::Priority)g.readEntry(QLatin1String("Priority_") + a->objectName(), (int)QAction::NormalPriority));
    }
}

void CentralWidget::saveState(KSharedConfigPtr cfg)
{
    KConfigGroup g = cfg->group("MainWindow");
    g.writeEntry("current_activity", currentIndex());

    const QList<QAction *> actions = activity_switching_group->actions();
    for (QAction *a : actions) {
        g.writeEntry(QLatin1String("Priority_") + a->objectName(), (int)a->priority());
    }
}

QAction *CentralWidget::addActivity(Activity *act)
{
    QAction *a = new QAction(QIcon::fromTheme(act->icon()), act->name(), this);
    // act->name() is i18n'ed, use <icon name, weight> as uniq id
    a->setObjectName(act->icon() + QLatin1String("_wght_") + QString::number(act->weight()));
    activity_switching_group->addAction(a);
    a->setCheckable(true);
    a->setToolTip(act->toolTip());
    a->setData(QVariant::fromValue(act));
    addWidget(act);
    return a;
}

void CentralWidget::removeActivity(Activity *act)
{
    const QList<QAction *> actions = activity_switching_group->actions();
    for (QAction *a : actions) {
        if (a->data().value<QObject *>() == act) {
            activity_switching_group->removeAction(a);
            a->deleteLater();
            break;
        }
    }
    removeWidget(act);
}

void CentralWidget::setCurrentActivity(Activity *act)
{
    setCurrentWidget(act);
}

Activity *CentralWidget::currentActivity()
{
    return (Activity *)currentWidget();
}

QList<QAction *> CentralWidget::activitySwitchingActions()
{
    return activity_switching_group->actions();
}

void CentralWidget::switchActivity(QAction *action)
{
    for (int i = 0; i < count(); i++) {
        Activity *act = (Activity *)widget(i);
        if (action->data().value<QObject *>() == act) {
            changeActivity(act);
            break;
        }
    }
}

}
