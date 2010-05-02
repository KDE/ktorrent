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

#include <QWidget>
#include <QStackedWidget>
#include <QBoxLayout>
#include <KSharedConfig>
#include <ktcore_export.h>
#include "activitybar.h"

namespace kt 
{
	
	/**
	 * The CentralWidget holds the ActivityBar and widget stack. The central widget can rearrange them, so
	 * that the ActivityBar can be placed on the left, right, top or bottom edge.
	 */
	class KTCORE_EXPORT CentralWidget : public QWidget
	{
		Q_OBJECT
	public:
		CentralWidget(QWidget* parent);
		virtual ~CentralWidget();
		
		ActivityBar* activityBar() {return activity_bar;}
		void loadState(KSharedConfigPtr cfg);
		void saveState(KSharedConfigPtr cfg);
		
	private slots:
		void setActivityBarPosition(ActivityListPosition p);
		
	private:
		ActivityBar* activity_bar;
		QStackedWidget* stack;
		QBoxLayout* layout;
		ActivityListPosition pos;
	};

}

#endif // KT_CENTRALWIDGET_H
