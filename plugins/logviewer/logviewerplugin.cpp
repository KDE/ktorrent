/*
    SPDX-FileCopyrightText: 2005-2007 Joris Guisson <joris.guisson@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QDockWidget>

#include <KLocalizedString>
#include <KMainWindow>
#include <KPluginFactory>

#include "logflags.h"
#include "logprefpage.h"
#include "logviewer.h"
#include "logviewerplugin.h"
#include "logviewerpluginsettings.h"
#include <interfaces/coreinterface.h>
#include <interfaces/guiinterface.h>
#include <interfaces/torrentactivityinterface.h>
#include <torrent/globals.h>
#include <util/log.h>

using namespace bt;

K_PLUGIN_CLASS_WITH_JSON(kt::LogViewerPlugin, "ktorrent_logviewer.json")

namespace kt
{
LogViewerPlugin::LogViewerPlugin(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : Plugin(parent, data, args)
{
}

LogViewerPlugin::~LogViewerPlugin()
{
}

void LogViewerPlugin::load()
{
    connect(getCore(), &CoreInterface::settingsChanged, this, &LogViewerPlugin::applySettings);
    flags = new LogFlags();
    lv = new LogViewer(flags);
    pref = new LogPrefPage(flags, nullptr);

    pos = (LogViewerPosition)LogViewerPluginSettings::logWidgetPosition();
    addLogViewerToGUI();
    getGUI()->addPrefPage(pref);
    AddLogMonitor(lv);
    applySettings();
}

void LogViewerPlugin::unload()
{
    pref->saveState();
    disconnect(getCore(), &CoreInterface::settingsChanged, this, &LogViewerPlugin::applySettings);
    getGUI()->removePrefPage(pref);
    removeLogViewerFromGUI();
    RemoveLogMonitor(lv);
    delete lv;
    lv = nullptr;
    delete pref;
    pref = nullptr;
    delete flags;
    flags = nullptr;
}

void LogViewerPlugin::applySettings()
{
    lv->setRichText(LogViewerPluginSettings::useRichText());
    lv->setMaxBlockCount(LogViewerPluginSettings::maxBlockCount());
    LogViewerPosition p = (LogViewerPosition)LogViewerPluginSettings::logWidgetPosition();
    if (pos != p) {
        removeLogViewerFromGUI();
        pos = p;
        addLogViewerToGUI();
    }
}

void LogViewerPlugin::addLogViewerToGUI()
{
    switch (pos) {
    case SEPARATE_ACTIVITY:
        getGUI()->addActivity(lv);
        break;
    case DOCKABLE_WIDGET: {
        KMainWindow *mwnd = getGUI()->getMainWindow();
        dock = new QDockWidget(mwnd);
        dock->setWidget(lv);
        dock->setObjectName(QStringLiteral("LogViewerDockWidget"));
        mwnd->addDockWidget(Qt::BottomDockWidgetArea, dock);
        break;
    }
    case TORRENT_ACTIVITY:
        getGUI()->getTorrentActivity()->addToolWidget(lv, lv->name(), lv->icon(), lv->toolTip());
        break;
    }
}

void LogViewerPlugin::removeLogViewerFromGUI()
{
    switch (pos) {
    case SEPARATE_ACTIVITY:
        getGUI()->removeActivity(lv);
        break;
    case TORRENT_ACTIVITY:
        getGUI()->getTorrentActivity()->removeToolWidget(lv);
        break;
    case DOCKABLE_WIDGET: {
        KMainWindow *mwnd = getGUI()->getMainWindow();
        mwnd->removeDockWidget(dock);
        dock->setWidget(nullptr);
        lv->setParent(nullptr);
        delete dock;
        dock = nullptr;
        break;
    }
    }
}

void LogViewerPlugin::guiUpdate()
{
    if (lv)
        lv->processPending();
}

}
#include "logviewerplugin.moc"
