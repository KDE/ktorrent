/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#ifndef KTSCRIPTINGPLUGIN_H
#define KTSCRIPTINGPLUGIN_H

#include <interfaces/plugin.h>

class KJob;

namespace kt
{
	class ScriptManager;
	class ScriptModel;
	
	/**
		@author
	*/
	class ScriptingPlugin : public Plugin
	{
		Q_OBJECT
	public:
		ScriptingPlugin(QObject* parent, const QStringList& args);
		virtual ~ScriptingPlugin();

		virtual void load();
		virtual void unload();
		virtual bool versionCheck(const QString& version) const;
	private:
		void setupActions();;
		void scriptDownloadFinished(KJob* job);
		void loadScripts();
		void saveScripts();
		
	private slots:
		void addScript();
		void removeScript();
		void runScript();
		void stopScript();
		void editScript();
		
	private:
		ScriptManager* sman;
		ScriptModel* model;
		KAction* add_script;
		KAction* remove_script;
		KAction* run_script;
		KAction* stop_script;
		KAction* edit_script;
	};

}

#endif
