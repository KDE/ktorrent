/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QFileDialog>

#include <KActionCollection>
#include <KConfigGroup>
#include <KIO/CopyJob>
#include <KLocalizedString>
#include <KMainWindow>
#include <KMessageBox>
#include <KPluginFactory>
#include <Kross/Core/ActionCollection>
#include <Kross/Core/Interpreter>
#include <Kross/Core/Manager>

#include <dbus/dbus.h>
#include <interfaces/coreinterface.h>
#include <interfaces/functions.h>
#include <interfaces/guiinterface.h>
#include <util/error.h>
#include <util/fileops.h>
#include <util/functions.h>
#include <util/log.h>
#include <util/logsystemmanager.h>

#include "api/scriptingmodule.h"
#include "script.h"
#include "scriptingplugin.h"
#include "scriptmanager.h"
#include "scriptmodel.h"

K_PLUGIN_FACTORY_WITH_JSON(ktorrent_scripting, "ktorrent_scripting.json", registerPlugin<kt::ScriptingPlugin>();)

using namespace bt;

namespace kt
{
ScriptingPlugin::ScriptingPlugin(QObject *parent, const QVariantList &args)
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
    QString script_dir = kt::DataDir() + QStringLiteral("scripts") + bt::DirSeparator();
    if (!bt::Exists(script_dir))
        bt::MakeDir(script_dir, true);

    LogSystemManager::instance().registerSystem(i18n("Scripting"), SYS_SCR);
    model = new ScriptModel(this);
    // add the KTorrent object
    Kross::Manager::self().addObject(getCore()->getExternalInterface(), QStringLiteral("KTorrent"));
    Kross::Manager::self().addObject(new ScriptingModule(getGUI(), getCore(), this), QStringLiteral("KTScriptingPlugin"));
    loadScripts();

    Out(SYS_SCR | LOG_DEBUG) << "Supported interpreters : " << endl;
    const QStringList interpreters = Kross::Manager::self().interpreters();
    for (const QString &s : interpreters)
        Out(SYS_SCR | LOG_DEBUG) << s << endl;

    sman = new ScriptManager(model, nullptr);
    connect(sman, &ScriptManager::addScript, this, &ScriptingPlugin::addScript);
    connect(sman, &ScriptManager::removeScript, this, &ScriptingPlugin::removeScript);
    connect(model, &ScriptModel::showPropertiesDialog, sman, qOverload<Script *>(&ScriptManager::showProperties));
    getGUI()->addActivity(sman);
}

void ScriptingPlugin::unload()
{
    LogSystemManager::instance().unregisterSystem(i18n("Scripting"));
    // save currently loaded scripts
    saveScripts();
    getGUI()->removeActivity(sman);
    delete sman;
    sman = nullptr;
    delete model;
    model = nullptr;
}

void ScriptingPlugin::loadScripts()
{
    const QStringList dir_list =
        QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("ktorrent/scripts"), QStandardPaths::LocateDirectory);
    for (const QString &dir : dir_list) {
        QDir d(dir);
        const QStringList subdirs = d.entryList(QDir::Dirs);
        for (const QString &sdir : subdirs) {
            if (sdir != QStringLiteral("..") && sdir != QStringLiteral(".")) {
                QString absolute_path = d.absoluteFilePath(sdir);
                Script *s = loadScriptDir(absolute_path);
                if (s) {
                    // Scripts in the home directory can be deleted
                    s->setRemovable(absolute_path.startsWith(kt::DataDir()));
                }
            }
        }
    }

    //
    KConfigGroup g = KSharedConfig::openConfig()->group(QLatin1String("Scripting"));
    const QStringList scripts = g.readEntry(QLatin1String("scripts"), QStringList());
    for (const QString &s : scripts) {
        Out(SYS_SCR | LOG_DEBUG) << "Loading script " << s << endl;
        if (bt::Exists(s)) {
            try {
                model->addScript(s);
            } catch (bt::Error &err) {
                getGUI()->errorMsg(err.toString());
            }
        }
    }

    // Start scripts which where running the previous time
    QStringList running = g.readEntry("running", QStringList());
    if (running.count())
        model->runScripts(running);
}

Script *ScriptingPlugin::loadScriptDir(const QString &dir)
{
    QDir d(dir);
    const QStringList files = d.entryList(QDir::Files);
    QString desktop_file;
    QString dir_path = dir;
    if (!dir_path.endsWith(bt::DirSeparator()))
        dir_path.append(bt::DirSeparator());

    // look for desktop files
    for (const QString &file : files) {
        if (file.endsWith(QStringLiteral(".desktop"))) {
            return model->addScriptFromDesktopFile(dir_path, file);
        }
    }

    return nullptr;
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
    QString filter = QStringLiteral("*.tar.gz *.tar.bz2 *.zip | ") + i18n("KTorrent Script Packages") + QStringLiteral("\n *.rb *.py *.js | ") + i18n("Scripts")
        + QStringLiteral("\n* |") + i18n("All files");

    QUrl url = QFileDialog::getOpenFileUrl(getGUI()->getMainWindow(), QString(), QUrl(QStringLiteral("kfiledialog:///addScript")), filter);
    if (!url.isValid())
        return;

    try {
        if (url.isLocalFile()) {
            model->addScript(url.toLocalFile());
        } else {
            QString script_dir = kt::DataDir() + QStringLiteral("scripts") + bt::DirSeparator();
            KIO::CopyJob *j = KIO::copy(url, QUrl::fromLocalFile(script_dir + url.fileName()));
            connect(j, &KIO::CopyJob::result, this, &ScriptingPlugin::scriptDownloadFinished);
        }
    } catch (bt::Error &err) {
        getGUI()->errorMsg(err.toString());
    }
}

void ScriptingPlugin::scriptDownloadFinished(KJob *job)
{
    KIO::CopyJob *j = (KIO::CopyJob *)job;
    if (j->error()) {
        getGUI()->errorMsg(j);
    } else {
        try {
            QString script_dir = kt::DataDir() + QStringLiteral("scripts") + bt::DirSeparator();
            model->addScript(script_dir + j->destUrl().fileName());
        } catch (bt::Error &err) {
            getGUI()->errorMsg(err.toString());
        }
    }
}

void ScriptingPlugin::removeScript()
{
    QStringList scripts_to_delete;
    const QModelIndexList indices = sman->selectedScripts();
    for (const QModelIndex &idx : indices) {
        Script *s = model->scriptForIndex(idx);
        if (s && !s->packageDirectory().isEmpty())
            scripts_to_delete.append(s->name());
    }

    if (scripts_to_delete.count() > 0) {
        QString msg = i18n(
            "Removing these scripts will fully delete them from your disk. "
            "Are you sure you want to do this?");
        if (KMessageBox::questionYesNoList(getGUI()->getMainWindow(), msg, scripts_to_delete) != KMessageBox::Yes)
            return;
    }

    model->removeScripts(indices);
    saveScripts();
    sman->updateActions(sman->selectedScripts());
}

bool ScriptingPlugin::versionCheck(const QString &version) const
{
    return version == QStringLiteral(VERSION);
}
}

#include "scriptingplugin.moc"
