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
#include <krun.h>
#include <kgenericfactory.h>
#include <kactioncollection.h>
#include <kfiledialog.h>
#include <kmainwindow.h>
#include <kstandarddirs.h>
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
#include <util/logsystemmanager.h>

#include "scriptingplugin.h"
#include "scriptmanager.h"
#include "scriptmodel.h"
#include "script.h"
#include "ui_scriptproperties.h"
#include "api/scriptingmodule.h"

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
		connect(remove_script,SIGNAL(triggered()),this,SLOT(removeScript()));
		ac->addAction("remove_script",remove_script);
		
		run_script = new KAction(KIcon("system-run"),i18n("Run Script"),this);
		connect(run_script,SIGNAL(triggered()),this,SLOT(runScript()));
		ac->addAction("run_script",run_script);
		
		stop_script = new KAction(KIcon("media-playback-stop"),i18n("Stop Script"),this);
		connect(stop_script,SIGNAL(triggered()),this,SLOT(stopScript()));
		ac->addAction("stop_script",stop_script);
		
		edit_script = new KAction(KIcon("document-open"),i18n("Edit Script"),this);
		connect(edit_script,SIGNAL(triggered()),this,SLOT(editScript()));
		ac->addAction("edit_script",edit_script);
		
		properties = new KAction(KIcon("dialog-information"),i18n("Properties"),this);
		connect(properties,SIGNAL(triggered()),this,SLOT(showProperties()));
		ac->addAction("script_properties",properties);
		
		configure_script = new KAction(KIcon("preferences-other"),i18n("Configure"),this);
		connect(configure_script,SIGNAL(triggered()),this,SLOT(configureScript()));
		ac->addAction("configure_script",configure_script);
	}

	void ScriptingPlugin::load()
	{
		LogSystemManager::instance().registerSystem(i18n("Scripting"),SYS_SCR);
		model = new ScriptModel(this);
		// add the KTorrent object
		Kross::Manager::self().addObject(getCore()->getExternalInterface(),"KTorrent");
		Kross::Manager::self().addObject(new ScriptingModule(this),"KTScriptingPlugin");
		loadScripts();
		
		Out(SYS_SCR|LOG_DEBUG) << "Supported interpreters : " << endl;
		QStringList interpreters = Kross::Manager::self().interpreters();
		foreach (const QString & s,interpreters)
			Out(SYS_SCR|LOG_DEBUG) << s << endl;
		
		setupActions();
		sman = new ScriptManager(model,actionCollection(),0);
		getGUI()->addToolWidget(sman,"text-x-script",i18n("Scripts"),GUIInterface::DOCK_LEFT);
	}

	void ScriptingPlugin::unload()
	{
		LogSystemManager::instance().unregisterSystem(i18n("Scripting"));
		// save currently loaded scripts
		saveScripts();
		getGUI()->removeToolWidget(sman);
		delete sman;
		sman = 0;
		delete model;
		model = 0;
	}
	
	void ScriptingPlugin::loadScripts()
	{
		QStringList dir_list = KGlobal::dirs()->findDirs("data", "ktorrent/scripts");
		foreach (const QString & dir,dir_list)
		{
			QDir d(dir);
			QStringList subdirs = d.entryList(QDir::Dirs);
			foreach (const QString & sdir,subdirs)
			{
				if (sdir != ".." && sdir != ".")
				{
					Script* s = loadScriptDir(d.absoluteFilePath(sdir));
					if (s)
						s->setRemoveable(false);
				}
			}
		}
		
		// 
		KConfigGroup g = KGlobal::config()->group("Scripting");
		QStringList scripts = g.readEntry("scripts",QStringList());
		foreach (const QString & s,scripts)
		{
			Out(SYS_SCR|LOG_DEBUG) << "Loading script " << s << endl;
			if (bt::Exists(s))
				model->addScript(s);
		}
	}
	
	Script* ScriptingPlugin::loadScriptDir(const QString & dir)
	{
		QDir d(dir);
		QStringList files = d.entryList(QDir::Files);
		QString desktop_file;
		QString dir_path = dir;
		if (!dir_path.endsWith(bt::DirSeparator()))
			dir_path.append(bt::DirSeparator());
		
		// look for desktop files
		foreach (const QString & file,files)
		{
			if (file.endsWith(".desktop"))
			{
				return model->addScriptFromDesktopFile(dir_path,file);
			}
		}
		
		return 0;
	}
	
	void ScriptingPlugin::saveScripts()
	{
		KConfigGroup g = KGlobal::config()->group("Scripting");
		g.writeEntry("scripts",model->scriptFiles());
		g.sync();
	}
	
	void ScriptingPlugin::addScript()
	{
		QString filter = "*.rb *.py *.js | " + i18n("Scripts") + "\n* |" + i18n("All files");
		KUrl url = KFileDialog::getOpenUrl(KUrl("kfiledialog:///addScript"),filter,getGUI()->getMainWindow());
		if (!url.isValid())
			return;

		if (url.isLocalFile())
		{
			model->addScript(url.pathOrUrl());
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
			model->addScript(script_dir + j->destUrl().fileName());
		}
	}
	
	void ScriptingPlugin::removeScript()
	{
		model->removeScripts(sman->selectedScripts());
		saveScripts();
		sman->updateActions(sman->selectedScripts());
	}
	
	void ScriptingPlugin::runScript()
	{
		QModelIndexList sel = sman->selectedScripts();
		foreach (const QModelIndex & idx,sel)
		{
			if (!model->setData(idx,Qt::Checked,Qt::CheckStateRole))
				Out(SYS_SCR|LOG_DEBUG) << "setData failed" << endl;
		}
		sman->updateActions(sel);
	}
	
	void ScriptingPlugin::stopScript()
	{
		QModelIndexList sel = sman->selectedScripts();
		foreach (const QModelIndex & idx,sel)
		{
			if (!model->setData(idx,Qt::Unchecked,Qt::CheckStateRole))
				Out(SYS_SCR|LOG_DEBUG) << "setData failed" << endl;
		}
		sman->updateActions(sel);
	}
	
	void ScriptingPlugin::editScript()
	{
		QModelIndexList sel = sman->selectedScripts();
		foreach (const QModelIndex & idx,sel)
		{
			Script* s = model->scriptForIndex(idx);
			if (s)
				new KRun(KUrl(s->scriptFile()), 0, 0, true, true);
		}
		
	}
	
	void ScriptingPlugin::showProperties()
	{
		QModelIndexList sel = sman->selectedScripts();
		if (sel.count() != 1)
			return;
		
		Script* s = model->scriptForIndex(sel.front());
		if (!s || !s->metaInfo().valid())
			return;
			
		Ui_ScriptProperties prop;
		KDialog* dialog = new KDialog(sman);
		dialog->setButtons(KDialog::Ok);
		dialog->setWindowTitle(i18n("Script Properties"));
		prop.setupUi(dialog->mainWidget());
		prop.m_icon->setPixmap(DesktopIcon(s->iconName()));
		prop.m_name->setText(s->name());
		prop.m_description->setText(s->metaInfo().comment);
		prop.m_author->setText(s->metaInfo().author);
		prop.m_license->setText(s->metaInfo().license);
		prop.m_email->setText(s->metaInfo().email);
		prop.m_website->setText(s->metaInfo().website);
		dialog->exec();
		delete dialog;
	}
	
	void ScriptingPlugin::configureScript()
	{
		QModelIndexList sel = sman->selectedScripts();
		if (sel.count() != 1)
			return;
		
		Script* s = model->scriptForIndex(sel.front());
		if (!s || !s->metaInfo().valid() || !s->hasConfigure())
			return;
		
		s->configure();
	}
	
	bool ScriptingPlugin::versionCheck(const QString & version) const
	{
		return version == KT_VERSION_MACRO;
	}
}
