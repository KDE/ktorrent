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
#ifndef KTschedulerPLUGIN_H
#define KTschedulerPLUGIN_H

#include <interfaces/plugin.h>

#include "schedulerprefpage.h"

#include <qtimer.h>

#include <kstdaction.h>

class QString;


namespace kt
{	
	class BWSPrefPage;
	
	/**
	 * @author Ivan Vasic <ivasic@gmail.com>
	 * @brief KTorrent scheduler plugin.
	 *
	 */
	class SchedulerPlugin : public Plugin
	{
		Q_OBJECT
	public:
		SchedulerPlugin(QObject* parent, const char* name, const QStringList& args);
		virtual ~SchedulerPlugin();

		virtual void load();
		virtual void unload();
		virtual bool versionCheck(const QString& version) const;
		
		void updateEnabledBWS();
		
	public slots:
		void timer_triggered();
		void openBWS();
	
	private:
		QTimer m_timer;
		
		/* BANDWIDTH SCHEDULE PLUGIN */
 		SchedulerPrefPage* Pref;
		KAction* bws_action;
	};

}

#endif
