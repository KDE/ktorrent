/*
    SPDX-FileCopyrightText: 2009 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KT_CENTRALWIDGET_H
#define KT_CENTRALWIDGET_H

#include <KSharedConfig>
#include <QAction>
#include <QActionGroup>
#include <QStackedWidget>

#include <ktcore_export.h>

namespace kt
{
class Activity;

/**
 * The CentralWidget holds the widget stack.
 */
class KTCORE_EXPORT CentralWidget : public QStackedWidget
{
    Q_OBJECT
public:
    CentralWidget(QWidget *parent);
    ~CentralWidget() override;

    /// Add an Activity
    QAction *addActivity(Activity *act);

    /// Remove an Activity (doesn't delete it)
    void removeActivity(Activity *act);

    /// Set the current activity
    void setCurrentActivity(Activity *act);

    /// Get the current activity
    Activity *currentActivity();

    /// Load the state of the widget
    void loadState(KSharedConfigPtr cfg);

    /// Save the state of the widget
    void saveState(KSharedConfigPtr cfg);

    /// Get the list of actions to switch between activities
    QList<QAction *> activitySwitchingActions();

private Q_SLOTS:
    void switchActivity(QAction *action);

Q_SIGNALS:
    /// Emitted when the current Activity needs to be changed
    void changeActivity(Activity *act);

private:
    QActionGroup *activity_switching_group;
};

}

#endif // KT_CENTRALWIDGET_H
