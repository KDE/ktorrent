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
#include <qthread.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qregexp.h>
#include <qvalidator.h>
#include <qlayout.h>
#include <qdialog.h>
#include <qobject.h>

#include <kapplication.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <kio/job.h>
#include <kio/jobuidelegate.h>

#include <util/log.h>
#include <util/functions.h>
#include <util/fileops.h>
#include <util/error.h>
#include <interfaces/functions.h>
#include <interfaces/coreinterface.h>


#include "ipblockingprefpage.h"
#include "ipfilterpluginsettings.h"
#include "ipfilterplugin.h"
#include "convertdialog.h"


using namespace bt;

#define MAX_RANGES 500

namespace kt
{

	IPBlockingPrefPage::IPBlockingPrefPage(CoreInterface* core, IPFilterPlugin* p)
	: PrefPageInterface(IPBlockingPluginSettings::self(),i18n("IP Filter"), "view-filter",0), m_core(core), m_plugin(p)
	{
		setupUi(this);
		connect(kcfg_useLevel1,SIGNAL(toggled(bool)),this,SLOT(checkUseLevel1Toggled(bool)));
		connect(m_download,SIGNAL(clicked()),this,SLOT(btnDownloadClicked()));
		
	}

	IPBlockingPrefPage::~IPBlockingPrefPage()
	{}
	
	void IPBlockingPrefPage::checkUseLevel1Toggled(bool check)
	{
		if(check)
		{
			kcfg_filterURL->setEnabled(true);
			m_download->setEnabled(true);
			m_plugin->loadAntiP2P();
		}
		else
		{
			m_status->setText("");
			kcfg_filterURL->setEnabled(false);
			m_download->setEnabled(false);
			m_plugin->unloadAntiP2P();
		}
		
		if (m_plugin->loadedAndRunning() && check)
			m_status->setText(i18n("Status: Loaded and running."));
		else
			m_status->setText(i18n("Status: Not loaded."));
	}
	
	void IPBlockingPrefPage::loadDefaults()
	{
		loadSettings();
	}
	
	void IPBlockingPrefPage::updateSettings()
	{
	}
	
	void IPBlockingPrefPage::loadSettings()
	{
		if (IPBlockingPluginSettings::useLevel1())
		{
			if (m_plugin->loadedAndRunning())
				m_status->setText(i18n("Status: Loaded and running."));
			else
				m_status->setText(i18n("Status: Not loaded."));
			
			kcfg_filterURL->setEnabled(true);
			m_download->setEnabled(true);
		}
		else
		{
			m_status->setText(i18n("Status: Not loaded."));
			kcfg_filterURL->setEnabled(false);
			m_download->setEnabled(false);
		}
	}
	
	void IPBlockingPrefPage::btnDownloadClicked()
	{
		KUrl url = kcfg_filterURL->url();
		
		QString temp = kt::DataDir() + "level1.tmp";
		if (bt::Exists(temp))
			bt::Delete(temp,true);
		
		// block GUI so you cannot do stuff during conversion
		m_download->setEnabled(false);
		m_status->setText(i18n("Status: Downloading and converting new block list ..."));
		kcfg_useLevel1->setEnabled(false);
		kcfg_filterURL->setEnabled(false);
		
		KJob* j = KIO::file_copy(url,temp,-1,KIO::HideProgressInfo|KIO::Overwrite);
		connect(j,SIGNAL(result(KJob*)),this,SLOT(downloadFileFinished(KJob*)));
	}
	
	void IPBlockingPrefPage::convert(KJob* j)
	{
		if (j->error())
		{
			((KIO::Job*)j)->ui()->showErrorMessage();
			restoreGUI();
		}
		else
			convert();
	}
	
	void IPBlockingPrefPage::downloadFileFinished(KJob* j)
	{
		if (j->error())
		{
			((KIO::Job*)j)->ui()->showErrorMessage();
			restoreGUI();
			return;
		}
		
		QString temp = kt::DataDir() + "level1.tmp";
		
		//now determine if it's ZIP or TXT file
		KMimeType::Ptr ptr = KMimeType::findByPath(temp);
		if (ptr->name() == "application/zip")
		{
			KJob* j2 = KIO::file_move(temp,kt::DataDir() + "level1.zip",-1,KIO::HideProgressInfo|KIO::Overwrite);
			connect(j2,SIGNAL(result(KJob*)),this,SLOT(extract(KJob*)));
		}
		else
		{
			KJob* j2 = KIO::file_move(temp,kt::DataDir() + "level1.txt",-1, KIO::HideProgressInfo|KIO::Overwrite);
			connect(j2,SIGNAL(result(KJob*)),this,SLOT(convert(KJob*)));
		}
	}
	
	void IPBlockingPrefPage::extract(KJob* j)
	{
		if (j->error())
		{
			((KIO::Job*)j)->ui()->showErrorMessage();
			restoreGUI();
			return;
		}
		
		KUrl zipfile("zip:" + kt::DataDir() + "level1.zip/splist.txt");
		KUrl destinationfile(kt::DataDir() + "level1.txt");
		KJob* j2 = KIO::file_copy(zipfile,destinationfile, -1, KIO::HideProgressInfo|KIO::Overwrite);
		connect(j2,SIGNAL(result(KJob*)),this,SLOT(convert(KJob*)));
	}
	
	void IPBlockingPrefPage::revertBackupFinished(KJob*)
	{
		m_plugin->loadAntiP2P();
		cleanUpFiles();
		restoreGUI();
	}
	
	void IPBlockingPrefPage::makeBackupFinished(KJob* j)
	{
		if (j && j->error())
		{
			((KIO::Job*)j)->ui()->showErrorMessage();
			restoreGUI();
		}
		else
		{
			m_plugin->unloadAntiP2P();
			
			ConvertDialog dlg(this);
			dlg.show();
			if (dlg.exec() == QDialog::Rejected)
			{
				// shit happened move back
				// make backup of data file, if stuff fails we can allways go back
				QString dat_file = kt::DataDir() + "level1.dat";
				QString tmp_file = kt::DataDir() + "level1.dat.tmp";
		
				if (bt::Exists(tmp_file))
				{
					KIO::Job* job = KIO::file_copy(tmp_file,dat_file,-1,KIO::HideProgressInfo|KIO::Overwrite);
					connect(job,SIGNAL(result(KJob*)),this,SLOT(revertBackupFinished(KJob*)));	
				}
				else
				{
					cleanUpFiles();
					restoreGUI();
				}
			}
			else
			{
				m_plugin->loadAntiP2P();
				cleanUpFiles();
				restoreGUI();
			}
		}
	}
	
	void IPBlockingPrefPage::convert()
	{
		if (bt::Exists(kt::DataDir() + "level1.dat"))
		{
			QString msg = i18n("Filter file (level1.dat) already exists, do you want to convert it again?");
			if((KMessageBox::questionYesNo(this,msg,i18n("File Exists")) == KMessageBox::No))
			{
				restoreGUI();
				return;
			}
			
			// make backup of data file, if stuff fails we can allways go back
			QString dat_file = kt::DataDir() + "level1.dat";
			QString tmp_file = kt::DataDir() + "level1.dat.tmp";
		
			
			KIO::Job* job = KIO::file_copy(dat_file,tmp_file,-1,KIO::HideProgressInfo|KIO::Overwrite);
			connect(job,SIGNAL(result(KJob*)),this,SLOT(makeBackupFinished(KJob*)));	
		}
		else
			makeBackupFinished(0);
	}
	
	void IPBlockingPrefPage::restoreGUI()
	{
		m_download->setEnabled(true);
		kcfg_useLevel1->setEnabled(true);
		kcfg_filterURL->setEnabled(true);
		
		if (m_plugin->loadedAndRunning())
			m_status->setText(i18n("Status: Loaded and running."));
		else
			m_status->setText(i18n("Status: Not loaded."));
	}
	
	void IPBlockingPrefPage::cleanUpFiles()
	{
		// cleanup temp files
		cleanUp(kt::DataDir() + "level1.zip");
		cleanUp(kt::DataDir() + "level1.txt");
		cleanUp(kt::DataDir() + "level1.tmp");
		cleanUp(kt::DataDir() + "level1.dat.tmp");
	}
	
	void IPBlockingPrefPage::cleanUp(const QString & path)
	{
		if (bt::Exists(path))
			bt::Delete(path,true);
	}
}
#include "ipblockingprefpage.moc"
