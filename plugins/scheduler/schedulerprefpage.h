/***************************************************************************
 *   Copyright (C) 2006 by Ivan Vasić                                      *
 *   ivasic@gmail.com                                                      *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.           *
 ***************************************************************************/
 
#ifndef KTSchedulerPREFPAGE_H
#define KTSchedulerPREFPAGE_H

#include <interfaces/prefpageinterface.h>

#include "schedulerprefpagewidget.h"

namespace kt
{
	class SchedulerPlugin;
	
	/**
	 * @brief Scheduler Preferences Page.
	 * @author Ivan Vasic <ivasic@gmail.com>
	*/
	class SchedulerPrefPage : public PrefPageInterface
	{
		public:
			SchedulerPrefPage(SchedulerPlugin* plugin);
			virtual ~SchedulerPrefPage();
			
			virtual bool apply();
			virtual void createWidget(QWidget* parent);
			virtual void updateData();
			virtual void deleteWidget();
			
		private:
			SchedulerPlugin* m_plugin;
			SchedulerPrefPageWidget* widget;
	};
}

#endif
