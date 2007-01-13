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
#ifndef KTSCANFOLDERPLUGIN_H
#define KTSCANFOLDERPLUGIN_H

#include <interfaces/plugin.h>

class QString;


namespace kt
{	
	class ScanFolder;
	class ScanFolderPrefPage;
	
	/**
	 * @author Ivan Vasic <ivasic@gmail.com>
	 * @brief KTorrent ScanFolder plugin
	 * Automatically scans selected folder for torrent files and loads them.
	 */
	class ScanFolderPlugin : public Plugin
	{
		Q_OBJECT
	public:
		ScanFolderPlugin(QObject* parent, const char* name, const QStringList& args);
		virtual ~ScanFolderPlugin();

		virtual void load();
		virtual void unload();
		virtual bool versionCheck(const QString& version) const;
		
		void updateScanFolders();
		
		private:
			ScanFolder* m_sf1;
			ScanFolder* m_sf2;
			ScanFolder* m_sf3;
			
			ScanFolderPrefPage* pref;
	};

}

#endif
