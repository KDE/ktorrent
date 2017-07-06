/***************************************************************************
*   Copyright (C) 2009 by Joris Guisson                                   *
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

#ifndef KT_CENTRALWIDGET_H
#define KT_CENTRALWIDGET_H

#include <QActionGroup>
#include <QAction>
#include <QStackedWidget>
#include <KSharedConfig>

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
        CentralWidget(QWidget* parent);
        ~CentralWidget();

        /// Add an Activity
        QAction * addActivity(Activity* act);

        /// Remove an Activity (doesn't delete it)
        void removeActivity(Activity* act);

        /// Set the current activity
        void setCurrentActivity(Activity* act);

        /// Get the current activity
        Activity* currentActivity();

        /// Load the state of the widget
        void loadState(KSharedConfigPtr cfg);

        /// Save the state of the widget
        void saveState(KSharedConfigPtr cfg);

        /// Get the list of actions to switch between activities
        QList<QAction*> activitySwitchingActions();

    private slots:
        void switchActivity(QAction* action);

    signals:
        /// Emitted when the current Activity needs to be changed
        void changeActivity(Activity* act);

    private:
        QActionGroup* activity_switching_group;
    };

}

#endif // KT_CENTRALWIDGET_H
