/***************************************************************************
 *   Copyright (C) 2007 by Joris Guisson and Ivan Vasic                    *
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
#include <klocale.h>
#include <kconfigdialogmanager.h>

#include <interfaces/functions.h>
#include <interfaces/prefpageinterface.h>

#include "settings.h"
#include "prefdialog.h"
#include "core.h"
#include "ui_globalpref.h"
#include "ui_generalpref.h"
#include "advancedpref.h"

namespace kt
{
	class GlobalPref : public PrefPageInterface,public Ui_GlobalPref
	{
	public:
		GlobalPref(QWidget* parent) : PrefPageInterface(Settings::self(),i18n("Downloads"),"ktorrent",parent)
		{
			setupUi(this);
		}

		virtual ~GlobalPref() {}
	};

	class GeneralPref : public PrefPageInterface,public Ui_GeneralPref
	{
	public:
		GeneralPref(QWidget* parent) : PrefPageInterface(Settings::self(),i18n("General"),"configure",parent)
		{
			setupUi(this);
			kcfg_tempDir->setMode(KFile::Directory|KFile::ExistingOnly|KFile::LocalOnly);
			kcfg_saveDir->setMode(KFile::Directory|KFile::ExistingOnly|KFile::LocalOnly);
			kcfg_torrentCopyDir->setMode(KFile::Directory|KFile::ExistingOnly|KFile::LocalOnly);
			kcfg_completedDir->setMode(KFile::Directory|KFile::ExistingOnly|KFile::LocalOnly);
		}

		virtual ~GeneralPref()
		{
		}

		void loadSettings()
		{
			if (Settings::tempDir().path().length() == 0)
				kcfg_tempDir->setUrl(kt::DataDir());
			else
				kcfg_tempDir->setUrl(Settings::tempDir());

			kcfg_saveDir->setEnabled(Settings::useSaveDir());
			if (Settings::saveDir().path().length() == 0)
				kcfg_saveDir->setUrl(QDir::homePath());
			else
				kcfg_saveDir->setUrl(Settings::saveDir());

			kcfg_torrentCopyDir->setEnabled(Settings::useTorrentCopyDir());
			if (Settings::torrentCopyDir().path().length() == 0)
				kcfg_torrentCopyDir->setUrl(QDir::homePath());
			else
				kcfg_torrentCopyDir->setUrl(Settings::torrentCopyDir());

			kcfg_completedDir->setEnabled(Settings::useCompletedDir());
			if (Settings::completedDir().path().length() == 0)
				kcfg_completedDir->setUrl(QDir::homePath());
			else
				kcfg_completedDir->setUrl(Settings::completedDir());

			kcfg_downloadBandwidth->setEnabled(Settings::showSpeedBarInTrayIcon());
			kcfg_uploadBandwidth->setEnabled(Settings::showSpeedBarInTrayIcon());
			kcfg_allowUnencryptedConnections->setEnabled(Settings::useEncryption());
#ifdef ENABLE_DHT_SUPPORT
			kcfg_dhtPort->setEnabled(Settings::dhtSupport());
#else
			dhtGroupBox->setShown(false);
#endif
		}

		void loadDefaults()
		{
			loadSettings();
		}
	};

	


	PrefDialog::PrefDialog(QWidget* parent,Core* core) : KConfigDialog(parent,"settings",Settings::self())
	{
		KConfigDialogManager::propertyMap()->insert("KUrlRequester","url");
		setFaceType(KPageDialog::List);
		connect(this,SIGNAL(settingsChanged(const QString &)),core,SLOT(applySettings()));
		addPrefPage(new GlobalPref(this));
		addPrefPage(new GeneralPref(this));
		addPrefPage(new AdvancedPref(this));
	}

	PrefDialog::~PrefDialog()
	{
	}

	void PrefDialog::addPrefPage(PrefPageInterface* page)
	{
		KPageWidgetItem* p = addPage(page,page->config(),page->pageName(),page->pageIcon());
		pages.insert(page,p);
		if (!isHidden())
			page->loadSettings();
	}

	void PrefDialog::removePrefPage(PrefPageInterface* page)
	{
		KPageWidgetItem* p = pages.value(page);
		if (p)
		{
			removePage(p);
			pages.remove(page);
		}
	}

	void PrefDialog::updateWidgets()
	{
		foreach (PrefPageInterface* p,pages.keys())
			p->loadSettings();
	}

	void PrefDialog::updateWidgetsDefault()
	{
		foreach (PrefPageInterface* p,pages.keys())
			p->loadDefaults();
	}
	
	void PrefDialog::updateSettings()
	{
		foreach (PrefPageInterface* p,pages.keys())
			p->updateSettings();
	}

}

#include "prefdialog.moc"
