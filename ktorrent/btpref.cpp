/***************************************************************************
 *   Copyright (C) 2009 by Joris Guisson                                   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/
#include <klocale.h>
#include "btpref.h"
#include "settings.h"

namespace kt
{
	BTPref::BTPref(QWidget* parent): PrefPageInterface(Settings::self(),i18n("BitTorrent"),"application-x-bittorrent",parent)
	{
		setupUi(this);
		connect(kcfg_doUploadDataCheck,SIGNAL(toggled(bool)),this,SLOT(onUploadDataCheckToggled(bool)));
	}
	
	BTPref::~BTPref() 
	{
	}

	void BTPref::loadSettings()
	{
		kcfg_allowUnencryptedConnections->setEnabled(Settings::useEncryption());
		#ifdef ENABLE_DHT_SUPPORT
		kcfg_dhtPort->setEnabled(Settings::dhtSupport());
		#else
		kcfg_dhtPort->setEnabled(false);
		kcfg_dhtSupport->setEnabled(false);
		#endif
		kcfg_customIP->setEnabled(Settings::useCustomIP());
		
		kcfg_maxCorruptedBeforeRecheck->setEnabled(Settings::autoRecheck());
		kcfg_useMaxSizeForUploadDataCheck->setEnabled(Settings::doUploadDataCheck());
		kcfg_maxSizeForUploadDataCheck->setEnabled(Settings::doUploadDataCheck() && Settings::useMaxSizeForUploadDataCheck());
	}
	void BTPref::onUploadDataCheckToggled(bool on)
	{
		kcfg_useMaxSizeForUploadDataCheck->setEnabled(on);
		kcfg_maxSizeForUploadDataCheck->setEnabled(on&&kcfg_useMaxSizeForUploadDataCheck->isChecked());
	}
}
