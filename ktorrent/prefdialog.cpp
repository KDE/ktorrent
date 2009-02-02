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
#include "settings.h"
#include "prefdialog.h"
#include "core.h"
#include "generalpref.h"
#include "advancedpref.h"
#include "networkpref.h"
#include "proxypref.h"
#include "qmpref.h"
#include "recommendedsettingsdlg.h"

namespace kt
{
	

	
	
	PrefDialog::PrefDialog(QWidget* parent,Core* core) : KConfigDialog(parent,"settings",Settings::self())
	{
		KConfigDialogManager::propertyMap()->insert("KUrlRequester","url");
		setFaceType(KPageDialog::List);
		connect(this,SIGNAL(settingsChanged(const QString &)),core,SLOT(applySettings()));
		addPrefPage(new GeneralPref(this));
		net_pref = new NetworkPref(this);
		addPrefPage(net_pref);
		addPrefPage(new ProxyPref(this));
		addPrefPage(new BTPref(this));
		qm_pref = new QMPref(this);
		addPrefPage(qm_pref);
		addPrefPage(new AdvancedPref(this));
		
		connect(net_pref,SIGNAL(calculateRecommendedSettings()),this,SLOT(calculateRecommendedSettings()));
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
	
	void PrefDialog::updateWidgetsAndShow()
	{
		updateWidgets();
		show();
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
	
	void PrefDialog::calculateRecommendedSettings()
	{
		RecommendedSettingsDlg dlg(this);
		if (dlg.exec() == QDialog::Accepted)
		{
			qm_pref->kcfg_maxSeeds->setValue(dlg.max_seeds);
			qm_pref->kcfg_maxDownloads->setValue(dlg.max_downloads);
			qm_pref->kcfg_numUploadSlots->setValue(dlg.max_slots);
			net_pref->kcfg_maxDownloadRate->setValue(dlg.max_download_speed);
			net_pref->kcfg_maxUploadRate->setValue(dlg.max_upload_speed);
			net_pref->kcfg_maxConnections->setValue(dlg.max_conn_tor);
			net_pref->kcfg_maxTotalConnections->setValue(dlg.max_conn_glob);
		}
	}

}

#include "prefdialog.moc"

// kate: space-indent on; indent-width 8; replace-tabs off; mixed-indent off;
