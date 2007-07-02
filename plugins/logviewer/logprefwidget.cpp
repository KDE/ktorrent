/***************************************************************************
 *   Copyright (C) 2006 by Ivan Vasic                                      *
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
#include "logprefwidget.h"
#include "logviewerpluginsettings.h"
#include "logflags.h"

#include <kglobal.h>
#include <klocale.h>

#include <qwidget.h>
#include <qstring.h>
#include <qcheckbox.h>
#include <qcombobox.h>

namespace kt
{
	LogPrefWidget::LogPrefWidget(QWidget *parent, const char *name)
			:LogPrefWidgetBase(parent, name)
	{
		m_sysgen->setCurrentItem(getLevel(LogViewerPluginSettings::sysGEN()));
		m_syscon->setCurrentItem(getLevel(LogViewerPluginSettings::sysCON()));
		m_sysdht->setCurrentItem(getLevel(LogViewerPluginSettings::sysDHT()));
		m_systrk->setCurrentItem(getLevel(LogViewerPluginSettings::sysTRK()));
		m_sysdio->setCurrentItem(getLevel(LogViewerPluginSettings::sysDIO()));
		
		m_sysipf->setCurrentItem(getLevel(LogViewerPluginSettings::sysIPF()));
		m_syspfi->setCurrentItem(getLevel(LogViewerPluginSettings::sysPFI()));
		m_sysinw->setCurrentItem(getLevel(LogViewerPluginSettings::sysINW()));
		m_syspnp->setCurrentItem(getLevel(LogViewerPluginSettings::sysPNP()));
		m_syssrc->setCurrentItem(getLevel(LogViewerPluginSettings::sysSRC()));
		m_sysscd->setCurrentItem(getLevel(LogViewerPluginSettings::sysSCD()));
		m_syssnf->setCurrentItem(getLevel(LogViewerPluginSettings::sysSNF()));
		m_sysrss->setCurrentItem(getLevel(LogViewerPluginSettings::sysRSS()));
		m_sysweb->setCurrentItem(getLevel(LogViewerPluginSettings::sysWEB()));
		
		m_useRich->setChecked(LogViewerPluginSettings::useRichText());
	}
	
	bool LogPrefWidget::apply()
	{
		LogViewerPluginSettings::setSysGEN(getArg(m_sysgen->currentItem()));
		LogViewerPluginSettings::setSysCON(getArg(m_syscon->currentItem()));
		LogViewerPluginSettings::setSysDHT(getArg(m_sysdht->currentItem()));
		LogViewerPluginSettings::setSysTRK(getArg(m_systrk->currentItem()));
		LogViewerPluginSettings::setSysDIO(getArg(m_sysdio->currentItem()));
		
		LogViewerPluginSettings::setSysIPF(getArg(m_sysipf->currentItem()));
		LogViewerPluginSettings::setSysPFI(getArg(m_syspfi->currentItem()));
		LogViewerPluginSettings::setSysINW(getArg(m_sysinw->currentItem()));
		LogViewerPluginSettings::setSysPNP(getArg(m_syspnp->currentItem()));
		LogViewerPluginSettings::setSysSRC(getArg(m_syssrc->currentItem()));
		LogViewerPluginSettings::setSysSCD(getArg(m_sysscd->currentItem()));
		LogViewerPluginSettings::setSysSNF(getArg(m_syssnf->currentItem()));
		LogViewerPluginSettings::setSysRSS(getArg(m_sysrss->currentItem()));
		LogViewerPluginSettings::setSysWEB(getArg(m_sysweb->currentItem()));

		LogViewerPluginSettings::setUseRichText(m_useRich->isChecked());
		
		
		LogViewerPluginSettings::writeConfig();
		
		LogFlags::instance().updateFlags();
		
		return true;
	}
	
	int LogPrefWidget::getLevel(unsigned int arg)
	{
		switch(arg)
		{
			case 0x0F:
				return 0;
			case 0x07:
				return 1;
			case 0x03:
				return 2;
			case 0x01:
				return 3;
			case 0x00:
				return 4;
			default:
				return 0;
		}
	}
	
	unsigned int LogPrefWidget::getArg(int level)
	{
		switch(level)
		{
			case 0:
				return 0x0F;
			case 1:
				return 0x07;
			case 2:
				return 0x03;
			case 3:
				return 0x01;
			case 4:
				return 0x00;
			default:
				return 0;
		}
	}
}

#include "logprefwidget.moc"
