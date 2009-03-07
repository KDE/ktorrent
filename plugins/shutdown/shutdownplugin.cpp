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
#include <kjob.h>
#include <kgenericfactory.h>
#include <kworkspace.h>
#include <solid/control/powermanager.h>
#include "shutdownplugin.h"
#include "screensaver_interface.h"

K_EXPORT_COMPONENT_FACTORY(ktshutdownplugin,KGenericFactory<kt::ShutdownPlugin>("ktshutdownplugin"))

namespace kt
{
	ShutdownPlugin::ShutdownPlugin(QObject* parent, const QStringList& args) : Plugin(parent)
	{
		Q_UNUSED(args);
	}
	
	ShutdownPlugin::~ShutdownPlugin()
	{
	}

	bool ShutdownPlugin::versionCheck(const QString& version) const
	{
		return version == KT_VERSION_MACRO;
	}

	void ShutdownPlugin::unload() 
	{
	}

	void ShutdownPlugin::load() 
	{
	}
	
	void ShutdownPlugin::shutdownComputer() 
	{
		KWorkSpace::requestShutDown(KWorkSpace::ShutdownConfirmNo,KWorkSpace::ShutdownTypeHalt);
	}
	
	void ShutdownPlugin::lock() 
	{
		QString interface("org.freedesktop.ScreenSaver");
		org::freedesktop::ScreenSaver screensaver(interface, "/ScreenSaver",QDBusConnection::sessionBus());
		screensaver.Lock();
	}
	
	void ShutdownPlugin::suspendToDisk() 
	{
		Solid::Control::PowerManager::SuspendMethod spdMethod = Solid::Control::PowerManager::ToDisk;
		KJob *job = Solid::Control::PowerManager::suspend(spdMethod);
		if (job != 0)
			job->start();
	}

	void ShutdownPlugin::suspendToRam() 
	{
		Solid::Control::PowerManager::SuspendMethod spdMethod = Solid::Control::PowerManager::ToRam;
		KJob *job = Solid::Control::PowerManager::suspend(spdMethod);
		if (job != 0)
			job->start();
	}
	
	void ShutdownPlugin::standby() 
	{
		Solid::Control::PowerManager::SuspendMethod spdMethod = Solid::Control::PowerManager::Standby;
		KJob *job = Solid::Control::PowerManager::suspend(spdMethod);
		if (job != 0)
			job->start();
	}



}
