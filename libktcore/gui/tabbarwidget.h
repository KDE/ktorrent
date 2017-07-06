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
        TabBarWidget(QSplitter* splitter, QWidget* parent);
        ~TabBarWidget();

        /// Add a tab to the TabBarWidget
        void addTab(QWidget* w, const QString& text, const QString& icon, const QString& tooltip);

        /// Remove a tab from the TabBarWidget
        void removeTab(QWidget* w);

        /// Save current status of sidebar, called at exit
        void saveState(KSharedConfigPtr cfg, const QString& group);

        /// Restore status from config, called at startup
        void loadState(KSharedConfigPtr cfg, const QString& group);

        /// Change the text of a tab
        void changeTabText(QWidget* w, const QString& text);

        /// Change the icon of a tab
        void changeTabIcon(QWidget* w, const QString& icon);

    private slots:
        void onActionTriggered(QAction* act);
        void toolButtonStyleChanged(Qt::ToolButtonStyle style);
        void setToolButtonStyle();

    private:
        void shrink();
        void unshrink();

    private:
        QToolBar* tab_bar;
        ActionGroup* action_group;
        QStackedWidget* widget_stack;
        bool shrunken;
        QMap<QWidget*, QAction*> widget_to_action;
    };

    class ActionGroup : public QObject
    {
        Q_OBJECT
    public:
        ActionGroup(QObject* parent = 0);
        ~ActionGroup();

        void addAction(QAction* act);
        void removeAction(QAction* act);

    private slots:
        void toggled(bool on);

    signals:
        void actionTriggered(QAction* a);

    private:
        QList<QAction*> actions;
    };
}

#endif // TABBARWIDGET_H
