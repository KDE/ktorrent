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

#include <QTimer>
#include <KAction>
#include <interfaces/plugin.h>
#include <interfaces/guiinterface.h>

class QString;
class KToolBar;


namespace kt
{	
	class ScheduleEditor;
	class Schedule;
	class BWPrefPage;
	
	/**
	 * @author Ivan Vasic <ivasic@gmail.com>
	 * @brief KTorrent scheduler plugin.
	 *
	 */
	class BWSchedulerPlugin : public Plugin,public CloseTabListener
	{
		Q_OBJECT
	public:
		BWSchedulerPlugin(QObject* parent, const QStringList& args);
		virtual ~BWSchedulerPlugin();

		virtual void load();
		virtual void unload();
		virtual bool versionCheck(const QString& version) const;
		virtual void tabCloseRequest(kt::GUIInterface* gui,QWidget* tab);
		
		
	public slots:
		void timerTriggered();
		void onLoaded(Schedule* ns);
		void onToggled(bool on);
		void colorsChanged();
	
	private:
		QTimer m_timer;
		KAction* m_bws_action;
		KToolBar* m_tool_bar;
		ScheduleEditor* m_editor;
		Schedule* m_schedule;
		BWPrefPage* m_pref;
	};

}

#endif
