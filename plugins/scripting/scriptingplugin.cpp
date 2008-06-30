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
#include <kgenericfactory.h>
#include <kactioncollection.h>
#include <kfiledialog.h>
#include <kmainwindow.h>
#include <kio/copyjob.h>
#include <kross/core/manager.h>
#include <kross/core/interpreter.h>
#include <kross/core/actioncollection.h>

#include <interfaces/guiinterface.h>
#include <interfaces/coreinterface.h>
#include <interfaces/functions.h>
#include <util/functions.h>
#include <util/fileops.h>
#include <util/error.h>
#include <util/log.h>

#include "scriptingplugin.h"
#include "scriptmanager.h"

K_EXPORT_COMPONENT_FACTORY(ktscriptingplugin,KGenericFactory<kt::ScriptingPlugin>("ktscriptingplugin"))
		
using namespace bt;

namespace kt
{

	ScriptingPlugin::ScriptingPlugin(QObject* parent, const QStringList& args)
			: Plugin(parent)
	{
		Q_UNUSED(args);
	}


	ScriptingPlugin::~ScriptingPlugin()
	{
	}
	
	void ScriptingPlugin::setupActions()
	{
		KActionCollection* ac = actionCollection();
		
		add_script = new KAction(KIcon("list-add"),i18n("Add Script"),this);
		connect(add_script,SIGNAL(triggered()),this,SLOT(addScript()));
		ac->addAction("add_script",add_script);
		
		remove_script = new KAction(KIcon("list-remove"),i18n("Remove Script"),this);
		connect(add_script,SIGNAL(triggered()),this,SLOT(removeScript()));
		ac->addAction("remove_script",remove_script);
	}

	void ScriptingPlugin::load()
	{
		// add the KTorrent object
		Kross::Manager::self().addObject(getCore()->getExternalInterface(),"KTorrent");
		loadScripts();
		
		Out(SYS_SCR|LOG_DEBUG) << "Supported interpreters : " << endl;
		QStringList interpreters = Kross::Manager::self().interpreters();
		foreach (const QString & s,interpreters)
			Out(SYS_SCR|LOG_DEBUG) << s << endl;
		
		setupActions();
		sman = new ScriptManager(actionCollection(),0);
		getGUI()->addToolWidget(sman,"text-x-script",i18n("Scripts"),GUIInterface::DOCK_LEFT);
	}

	void ScriptingPlugin::unload()
	{
		// save currently loaded scripts
		saveScripts();
		getGUI()->removeToolWidget(sman);
		delete sman;
		sman = 0;
	}
	
	void ScriptingPlugin::loadScripts()
	{
		KConfigGroup g = KGlobal::config()->group("Scripting");
		QStringList scripts = g.readEntry("scripts",QStringList());
		foreach (const QString & s,scripts)
		{
			Out(SYS_SCR|LOG_DEBUG) << "Loading script " << s << endl;
			if (bt::Exists(s))
				loadScript(s);
		}
	}
	
	void ScriptingPlugin::saveScripts()
	{
		QStringList scripts;
		
		QList<Kross::Action*> actions = Kross::Manager::self().actionCollection()->actions();
		foreach (Kross::Action* a,actions)
		{
			scripts.append(a->file());
		}
		
		KConfigGroup g = KGlobal::config()->group("Scripting");
		g.writeEntry("scripts",scripts);
	}
	
	void ScriptingPlugin::addScript()
	{
		QString filter = "*.rb *.py *.js | " + i18n("Scripts") + "\n* |" + i18n("All files");
		KUrl url = KFileDialog::getOpenUrl(KUrl("kfiledialog:///addScript"),filter,getGUI()->getMainWindow());
		if (!url.isValid())
			return;

		if (url.isLocalFile())
		{
			loadScript(url.pathOrUrl());
		}
		else
		{
			try
			{
				// make sure script dir exists
				QString script_dir = kt::DataDir() + "scripts" + bt::DirSeparator();
				if (!bt::Exists(script_dir))
					bt::MakeDir(script_dir);

				KIO::CopyJob* j = KIO::copy(url,KUrl(script_dir + url.fileName()));
				connect(j,SIGNAL(result(KJob*)),this,SLOT(scriptDownloadFinished( KJob* )));
			}
			catch (bt::Error & err)
			{
				getGUI()->errorMsg(err.toString());
			}
		}
	}
	
	void ScriptingPlugin::loadScript(const QString & file)
	{
		KMimeType::Ptr mt = KMimeType::findByPath(file);
		QString name = QFileInfo(file).fileName();
		Kross::Action* action = new Kross::Action(this,name);
		action->setText(name);
		action->setDescription(name);
		action->setFile(file);
		action->setIconName(mt->iconName());
		action->setInterpreter(Kross::Manager::self().interpreternameForFile(file));
		Kross::Manager::self().actionCollection()->addAction(file,action);
	}
	
	void ScriptingPlugin::scriptDownloadFinished(KJob* job)
	{
		KIO::CopyJob* j = (KIO::CopyJob*)job;
		if (j->error())
		{
			getGUI()->errorMsg(j);
		}
		else
		{
			QString script_dir = kt::DataDir() + "scripts" + bt::DirSeparator();
			loadScript(script_dir + j->destUrl().fileName());
		}
	}
	
	void ScriptingPlugin::removeScript()
	{
	}
	
	bool ScriptingPlugin::versionCheck(const QString & version) const
	{
		return version == KT_VERSION_MACRO;
	}
}
