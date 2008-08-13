/***************************************************************************
 *   Copyright (C) 2005 by Joris Guisson                                   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ***************************************************************************/
#ifndef IPBLOCKINGPREFPAGE_H
#define IPBLOCKINGPREFPAGE_H

#include <interfaces/prefpageinterface.h>
#include <interfaces/coreinterface.h>
#include <qthread.h>
#include "ui_ipblockingprefpage.h"
#include "ipfilterplugin.h"

class KProgress;
class KJob;

namespace kt
{
	class IPFilterPlugin;
	
	/**
	 * @author Ivan Vasic
	 * @brief IPBlocking plugin interface page
	 **/
	class IPBlockingPrefPage : public PrefPageInterface,public Ui_IPBlockingPrefPage
	{
		Q_OBJECT
	public:
		IPBlockingPrefPage(CoreInterface* core, IPFilterPlugin* p);
		virtual ~IPBlockingPrefPage();
		
	
		virtual void loadSettings();
		virtual void loadDefaults();
		virtual void updateSettings();
			
	private slots:
		void btnDownloadClicked();
		void checkUseLevel1Toggled(bool);
		void downloadFileFinished(KJob*);
		void convert(KJob*);
		void extract(KJob*);
		void makeBackupFinished(KJob* );
		void revertBackupFinished(KJob*);
		
		
	private:
		void convert();
		void cleanUp(const QString & path);
		void cleanUpFiles();
		void restoreGUI();

	private:
		CoreInterface* m_core;
		IPFilterPlugin* m_plugin;
	};
}
#endif
