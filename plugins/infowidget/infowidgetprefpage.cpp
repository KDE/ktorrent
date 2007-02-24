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
#include <klocale.h>
#include <qcheckbox.h>
#include <kglobal.h>
#include <kiconloader.h>
#include "infowidgetplugin.h"
#include "infowidgetprefpage.h"
#include "infowidgetpluginsettings.h"
#include "iwpref.h"


namespace kt
{

	InfoWidgetPrefPage::InfoWidgetPrefPage(InfoWidgetPlugin* iw)
	: PrefPageInterface(i18n("Info Widget"),i18n("Information Widget Options"),KGlobal::iconLoader()->loadIcon("ktinfowidget",KIcon::NoGroup)),iw(iw)
	{
		pref = 0;
	}


	InfoWidgetPrefPage::~InfoWidgetPrefPage()
	{}


	bool InfoWidgetPrefPage::apply()
	{
		InfoWidgetPluginSettings::setShowPeerView(pref->m_show_pv->isChecked());
		InfoWidgetPluginSettings::setShowChunkView(pref->m_show_cdv->isChecked());
		InfoWidgetPluginSettings::setShowTrackersView(pref->m_show_tv->isChecked());
		InfoWidgetPluginSettings::writeConfig();
		iw->showPeerView( InfoWidgetPluginSettings::showPeerView() );
		iw->showChunkView( InfoWidgetPluginSettings::showChunkView() );
		iw->showTrackerView( InfoWidgetPluginSettings::showTrackersView() );
		return true;
	}

	void InfoWidgetPrefPage::createWidget(QWidget* parent)
	{
		pref = new IWPref(parent);
		updateData();
	}

	void InfoWidgetPrefPage::deleteWidget()
	{
		delete pref;
	}

	void InfoWidgetPrefPage::updateData()
	{
		pref->m_show_pv->setChecked(InfoWidgetPluginSettings::showPeerView());
		pref->m_show_cdv->setChecked(InfoWidgetPluginSettings::showChunkView());
		pref->m_show_tv->setChecked(InfoWidgetPluginSettings::showTrackersView());
	}

}
