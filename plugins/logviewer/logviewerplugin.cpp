/***************************************************************************
 *   Copyright (C) 2005-2007 by Joris Guisson                              *
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

#include <QDockWidget>

#include <KPluginFactory>
#include <KLocalizedString>
#include <KMainWindow>

#include <util/log.h>
#include <torrent/globals.h>
#include <interfaces/guiinterface.h>
#include <interfaces/coreinterface.h>
#include <interfaces/torrentactivityinterface.h>
#include "logviewerplugin.h"
#include "logviewer.h"
#include "logprefpage.h"
#include "logflags.h"
#include "logviewerpluginsettings.h"


using namespace bt;

K_PLUGIN_FACTORY_WITH_JSON(ktorrent_logviewer, "ktorrent_logviewer.json", registerPlugin<kt::LogViewerPlugin>();)

namespace kt
{
    LogViewerPlugin::LogViewerPlugin(QObject* parent, const QVariantList&) : Plugin(parent)
        , lv(nullptr)
        , pref(nullptr)
        , flags(nullptr)
        , dock(nullptr)
        , pos(SEPARATE_ACTIVITY)
    {
    }


    LogViewerPlugin::~LogViewerPlugin()
    {}


    void LogViewerPlugin::load()
    {
        connect(getCore(), SIGNAL(settingsChanged()), this, SLOT(applySettings()));
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
        disconnect(getCore(), SIGNAL(settingsChanged()), this, SLOT(applySettings()));
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
        if (pos != p)
        {
            removeLogViewerFromGUI();
            pos = p;
            addLogViewerToGUI();
        }
    }

    void LogViewerPlugin::addLogViewerToGUI()
    {
        switch (pos)
        {
        case SEPARATE_ACTIVITY:
            getGUI()->addActivity(lv);
            break;
        case DOCKABLE_WIDGET:
        {
            KMainWindow* mwnd = getGUI()->getMainWindow();
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
        switch (pos)
        {
        case SEPARATE_ACTIVITY:
            getGUI()->removeActivity(lv);
            break;
        case TORRENT_ACTIVITY:
            getGUI()->getTorrentActivity()->removeToolWidget(lv);
            break;
        case DOCKABLE_WIDGET:
        {
            KMainWindow* mwnd = getGUI()->getMainWindow();
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

    bool LogViewerPlugin::versionCheck(const QString& version) const
    {
        return version == QStringLiteral(KT_VERSION_MACRO);
    }

}
#include "logviewerplugin.moc"
