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
#include <kstandarddirs.h>
#include <kmessagebox.h>
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
#include <dbus/dbus.h>

#include "scriptingplugin.h"
#include "scriptmanager.h"
#include "scriptmodel.h"
#include "script.h"
#include "api/scriptingmodule.h"

K_EXPORT_COMPONENT_FACTORY(ktscriptingplugin, KGenericFactory<kt::ScriptingPlugin>("ktscriptingplugin"))

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



    void ScriptingPlugin::load()
    {
        // make sure script dir exists
        QString script_dir = kt::DataDir() + "scripts" + bt::DirSeparator();
        if (!bt::Exists(script_dir))
            bt::MakeDir(script_dir, true);

        LogSystemManager::instance().registerSystem(i18n("Scripting"), SYS_SCR);
        model = new ScriptModel(this);
        // add the KTorrent object
        Kross::Manager::self().addObject(getCore()->getExternalInterface(), "KTorrent");
        Kross::Manager::self().addObject(new ScriptingModule(getGUI(), getCore(), this), "KTScriptingPlugin");
        loadScripts();

        Out(SYS_SCR | LOG_DEBUG) << "Supported interpreters : " << endl;
        QStringList interpreters = Kross::Manager::self().interpreters();
        foreach (const QString& s, interpreters)
            Out(SYS_SCR | LOG_DEBUG) << s << endl;

        sman = new ScriptManager(model, 0);
        connect(sman, SIGNAL(addScript()), this, SLOT(addScript()));
        connect(sman, SIGNAL(removeScript()), this, SLOT(removeScript()));
        connect(model, SIGNAL(showPropertiesDialog(Script*)), sman, SLOT(showProperties(Script*)));
        getGUI()->addActivity(sman);
    }

    void ScriptingPlugin::unload()
    {
        LogSystemManager::instance().unregisterSystem(i18n("Scripting"));
        // save currently loaded scripts
        saveScripts();
        getGUI()->removeActivity(sman);
        delete sman;
        sman = 0;
        delete model;
        model = 0;
    }

    void ScriptingPlugin::loadScripts()
    {
        QStringList dir_list = KGlobal::dirs()->findDirs("data", "ktorrent/scripts");
        foreach (const QString& dir, dir_list)
        {
            QDir d(dir);
            QStringList subdirs = d.entryList(QDir::Dirs);
            foreach (const QString& sdir, subdirs)
            {
                if (sdir != ".." && sdir != ".")
                {
                    QString absolute_path = d.absoluteFilePath(sdir);
                    Script* s = loadScriptDir(absolute_path);
                    if (s)
                    {
                        // Scripts in the home directory can be deleted
                        s->setRemoveable(absolute_path.startsWith(kt::DataDir()));
                    }
                }
            }
        }

        //
        KConfigGroup g = KSharedConfig::openConfig()->group("Scripting");
        QStringList scripts = g.readEntry("scripts", QStringList());
        foreach (const QString& s, scripts)
        {
            Out(SYS_SCR | LOG_DEBUG) << "Loading script " << s << endl;
            if (bt::Exists(s))
            {
                try
                {
                    model->addScript(s);
                }
                catch (bt::Error& err)
                {
                    getGUI()->errorMsg(err.toString());
                }
            }
        }

        // Start scripts which where running the previous time
        QStringList running = g.readEntry("running", QStringList());
        if (running.count())
            model->runScripts(running);
    }

    Script* ScriptingPlugin::loadScriptDir(const QString& dir)
    {
        QDir d(dir);
        QStringList files = d.entryList(QDir::Files);
        QString desktop_file;
        QString dir_path = dir;
        if (!dir_path.endsWith(bt::DirSeparator()))
            dir_path.append(bt::DirSeparator());

        // look for desktop files
        foreach (const QString& file, files)
        {
            if (file.endsWith(".desktop"))
            {
                return model->addScriptFromDesktopFile(dir_path, file);
            }
        }

        return 0;
    }

    void ScriptingPlugin::saveScripts()
    {
        KConfigGroup g = KSharedConfig::openConfig()->group("Scripting");
        g.writeEntry("scripts", model->scriptFiles());
        g.writeEntry("running", model->runningScriptFiles());
        g.sync();
    }

    void ScriptingPlugin::addScript()
    {
        QString filter = "*.tar.gz *.tar.bz2 *.zip | " + i18n("KTorrent Script Packages") +
                         "\n *.rb *.py *.js | " + i18n("Scripts") +
                         "\n* |" + i18n("All files");

        KUrl url = KFileDialog::getOpenUrl(KUrl("kfiledialog:///addScript"), filter, getGUI()->getMainWindow());
        if (!url.isValid())
            return;

        try
        {
            if (url.isLocalFile())
            {
                model->addScript(url.pathOrUrl());
            }
            else
            {
                QString script_dir = kt::DataDir() + "scripts" + bt::DirSeparator();
                KIO::CopyJob* j = KIO::copy(url, KUrl(script_dir + url.fileName()));
                connect(j, SIGNAL(result(KJob*)), this, SLOT(scriptDownloadFinished(KJob*)));
            }
        }
        catch (bt::Error& err)
        {
            getGUI()->errorMsg(err.toString());
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
            try
            {
                QString script_dir = kt::DataDir() + "scripts" + bt::DirSeparator();
                model->addScript(script_dir + j->destUrl().fileName());
            }
            catch (bt::Error& err)
            {
                getGUI()->errorMsg(err.toString());
            }
        }
    }

    void ScriptingPlugin::removeScript()
    {
        QStringList scripts_to_delete;
        QModelIndexList indices = sman->selectedScripts();
        foreach (const QModelIndex& idx, indices)
        {
            Script* s = model->scriptForIndex(idx);
            if (s && !s->packageDirectory().isEmpty())
                scripts_to_delete.append(s->name());
        }

        if (scripts_to_delete.count() > 0)
        {
            QString msg = i18n("Removing these scripts will fully delete them from your disk. "
                               "Are you sure you want to do this?");
            if (KMessageBox::questionYesNoList(getGUI()->getMainWindow(), msg, scripts_to_delete) != KMessageBox::Yes)
                return;
        }

        model->removeScripts(indices);
        saveScripts();
        sman->updateActions(sman->selectedScripts());
    }

    bool ScriptingPlugin::versionCheck(const QString& version) const
    {
        return version == KT_VERSION_MACRO;
    }
}
