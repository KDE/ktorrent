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
#include <qfile.h>
#include <qtextstream.h>
#include <klocale.h>
#include <kservicetypetrader.h>
#include <kparts/componentfactory.h>
#include <util/log.h>
#include <util/error.h>
#include <util/fileops.h>
#include <util/waitjob.h>
#include <torrent/globals.h>
#include <interfaces/guiinterface.h>
#include "pluginmanager.h"
#include "pluginmanagerprefpage.h"

using namespace bt;

namespace kt
{

	PluginManager::PluginManager(CoreInterface* core,GUIInterface* gui) : core(core),gui(gui)
	{
		prefpage = 0;
		loaded.setAutoDelete(true);
	}

	PluginManager::~PluginManager()
	{
		delete prefpage;
	}

	void PluginManager::loadPluginList()
	{
		KService::List offers = KServiceTypeTrader::self()->query("KTorrent/Plugin");
		plugins = KPluginInfo::fromServices(offers);

		for (KPluginInfo::List::iterator i = plugins.begin();i != plugins.end();i++)
		{	
			KPluginInfo & pi = *i;
			pi.setConfig(KGlobal::config()->group(pi.pluginName()));
			pi.load();
		}
		
		if (!prefpage)
		{
			prefpage = new PluginManagerPrefPage(this);
			gui->addPrefPage(prefpage);
		}
		prefpage->updatePluginList();
		
		loadPlugins();		
	}
	
	void PluginManager::loadPlugins()
	{
		int idx = 0;
		for (KPluginInfo::List::iterator i = plugins.begin();i != plugins.end();i++)
		{
			KPluginInfo & pi = *i;
			if (loaded.contains(idx) & !pi.isPluginEnabled())
			{
				// unload it
				unload(pi,idx);
				pi.save();
			}
			else if (!loaded.contains(idx) && pi.isPluginEnabled())
			{
				// load it
				load(pi,idx);
				pi.save();
			}
			idx++;
		}
	}
	
	void PluginManager::load(const KPluginInfo & pi,int idx)
	{
		KService::Ptr service = pi.service();
			
		Plugin* p = service->createInstance<kt::Plugin>(); 
		if (!p) 
        {
			p = service->createInstance<kt::Plugin>();
			if (!p)
			{
				Out(SYS_GEN|LOG_NOTICE) <<
						QString("Creating instance of plugin %1 failed !")
						.arg(service->library()) << endl;
				return;
			}
        }
			
		if (!p->versionCheck(kt::VERSION_STRING))
		{
			Out(SYS_GEN|LOG_NOTICE) <<
					QString("Plugin %1 version does not match KTorrent version, unloading it.")
					.arg(service->library()) << endl;

			delete p;
		}
		else
		{
			p->setCore(core);
			p->setGUI(gui);
			p->load();
			gui->mergePluginGui(p);
			p->loaded = true;
			loaded.insert(idx,p,true);
		}
	}
	
	void PluginManager::unload(const KPluginInfo & pi,int idx)
	{
		Plugin* p = loaded.find(idx);
		if (!p)
			return;
		
		// first shut it down properly
		bt::WaitJob* wjob = new WaitJob(2000);
		try
		{
			p->shutdown(wjob);
			if (wjob->needToWait())
				bt::WaitJob::execute(wjob);
			else
				delete wjob;
		}
		catch (Error & err)
		{
			Out(SYS_GEN|LOG_NOTICE) << "Error when unloading plugin: " << err.toString() << endl;
		}

		gui->removePluginGui(p);
		p->unload();
		p->loaded = false;
		loaded.erase(idx);
	}


	
	void PluginManager::unloadAll()
	{
		// first properly shutdown all plugins
		bt::WaitJob* wjob = new WaitJob(2000);
		try
		{
			bt::PtrMap<int,Plugin>::iterator i = loaded.begin();
			while (i != loaded.end())
			{
				Plugin* p = i->second;
				p->shutdown(wjob);
				i++;
			}
			if (wjob->needToWait())
				bt::WaitJob::execute(wjob);
			else
				delete wjob;
		}
		catch (Error & err)
		{
			Out(SYS_GEN|LOG_NOTICE) << "Error when unloading all plugins: " << err.toString() << endl;
		}
		
		// then unload them
		bt::PtrMap<int,Plugin>::iterator i = loaded.begin();
		while (i != loaded.end())
		{
			Plugin* p = i->second;
			gui->removePluginGui(p);
			p->unload();
			p->loaded = false;
			i++;
		}
		loaded.clear();
	}

	void PluginManager::updateGuiPlugins()
	{
		bt::PtrMap<int,Plugin>::iterator i = loaded.begin();
		while (i != loaded.end())
		{
			Plugin* p = i->second;
			p->guiUpdate();
			i++;
		}
	}

}
