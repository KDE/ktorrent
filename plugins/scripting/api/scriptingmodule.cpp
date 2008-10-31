/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
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
#include <kglobal.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <kstandarddirs.h>
#include <util/functions.h>
#include "scriptingmodule.h"

namespace kt
{

	ScriptingModule::ScriptingModule(QObject* parent)
			: QObject(parent)
	{
	}


	ScriptingModule::~ScriptingModule()
	{
	}

	QString ScriptingModule::scriptsDir() const
	{
		QStringList dirs = KGlobal::dirs()->findDirs("data", "ktorrent/scripts");
		if (dirs.count() == 0)
			return QString();
		
		QString ret = dirs.front();
		if (!ret.endsWith(bt::DirSeparator()))
			ret += bt::DirSeparator();
		
		return ret;
	}
	
	QString ScriptingModule::readConfigEntry(const QString & group,const QString & name,const QString & default_value)
	{
		KConfigGroup g = KGlobal::config()->group(group);
		return g.readEntry(name,default_value);
	}
	
	bool ScriptingModule::readConfigEntryBool(const QString & group,const QString & name,bool default_value)
	{
		KConfigGroup g = KGlobal::config()->group(group);
		return g.readEntry(name,default_value);
	}
	
	int ScriptingModule::readConfigEntryInt(const QString & group,const QString & name,int default_value)
	{
		KConfigGroup g = KGlobal::config()->group(group);
		return g.readEntry(name,default_value);
	}
	
	float ScriptingModule::readConfigEntryFloat(const QString & group,const QString & name,float default_value)
	{
		KConfigGroup g = KGlobal::config()->group(group);
		return g.readEntry(name,default_value);
	}
		
	void ScriptingModule::writeConfigEntry(const QString & group,const QString & name,const QString & value)
	{
		KConfigGroup g = KGlobal::config()->group(group);
		g.writeEntry(name,value);
	}
	
	void ScriptingModule::writeConfigEntryBool(const QString & group,const QString & name,bool value)
	{
		KConfigGroup g = KGlobal::config()->group(group);
		g.writeEntry(name,value);
	}
	
	void ScriptingModule::writeConfigEntryInt(const QString & group,const QString & name,int value)
	{
		KConfigGroup g = KGlobal::config()->group(group);
		g.writeEntry(name,value);
	}
	
	void ScriptingModule::writeConfigEntryFloat(const QString & group,const QString & name,float value)
	{
		KConfigGroup g = KGlobal::config()->group(group);
		g.writeEntry(name,value);
	}
		
	void ScriptingModule::syncConfig(const QString & group)
	{
		KConfigGroup g = KGlobal::config()->group(group);
		g.sync();
	}
}
