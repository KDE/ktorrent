/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#ifndef KTLOGVIEWERPLUGIN_H
#define KTLOGVIEWERPLUGIN_H

#include <interfaces/plugin.h>



namespace kt
{
	class LogViewer;
	class LogPrefPage;

	/**
	 * @author Joris Guisson
	*/
	class LogViewerPlugin : public Plugin
	{
		Q_OBJECT
	public:
		LogViewerPlugin(QObject* parent, const char* qt_name, const QStringList& args);
		virtual ~LogViewerPlugin();
			
		virtual void load();
		virtual void unload();
		virtual bool versionCheck(const QString& version) const;
	private:
		LogViewer* lv;
		LogPrefPage* pref;
	};

}

#endif
