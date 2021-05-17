/*
    SPDX-FileCopyrightText: 2020 Alexander Trufanov <trufanovan@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <interfaces/coreinterface.h>
#include <interfaces/guiinterface.h>
#include <interfaces/torrentactivityinterface.h>

#include <util/constants.h>
#include <util/functions.h>

#include <KLocalizedString>
#include <KMainWindow>
#include <KPluginFactory>
#include <QDockWidget>

#include "scanforlostfilesplugin.h"
#include "scanforlostfilespluginsettings.h"
#include "scanforlostfilesprefpage.h"
#include "scanforlostfileswidget.h"

using namespace bt;

K_PLUGIN_FACTORY_WITH_JSON(ktorrent_scanforlostfiles, "ktorrent_scanforlostfiles.json", registerPlugin<kt::ScanForLostFilesPlugin>();)

namespace kt
{
ScanForLostFilesPlugin::ScanForLostFilesPlugin(QObject *parent, const QVariantList &args)
    : Plugin(parent)
    , m_view(nullptr)
    , m_dock(nullptr)
    , m_pref(nullptr)
    , m_pos(SEPARATE_ACTIVITY)
{
    Q_UNUSED(args);
}

ScanForLostFilesPlugin::~ScanForLostFilesPlugin()
{
}

void ScanForLostFilesPlugin::load()
{
    m_view = new ScanForLostFilesWidget(this);
    m_pref = new ScanForLostFilesPrefPage(this, nullptr);
    m_pos = (SFLFPosition)ScanForLostFilesPluginSettings::scanForLostFilesWidgetPosition();

    addToGUI();
    getGUI()->addPrefPage(m_pref);
    connect(getCore(), &CoreInterface::settingsChanged, this, &ScanForLostFilesPlugin::updateScanForLostFiles);
    updateScanForLostFiles();
}

void ScanForLostFilesPlugin::unload()
{
    m_pref->saveSettings();
    disconnect(getCore(), &CoreInterface::settingsChanged, this, &ScanForLostFilesPlugin::updateScanForLostFiles);

    getGUI()->removePrefPage(m_pref);
    removeFromGUI();
    delete m_pref;
    m_pref = nullptr;
    delete m_view;
    m_view = nullptr;
}

void ScanForLostFilesPlugin::updateScanForLostFiles()
{
    SFLFPosition p = (SFLFPosition)ScanForLostFilesPluginSettings::scanForLostFilesWidgetPosition();
    if (m_pos != p) {
        removeFromGUI();
        m_pos = p;
        addToGUI();
    }
}

bool ScanForLostFilesPlugin::versionCheck(const QString &version) const
{
    return version == QStringLiteral(VERSION);
}

void ScanForLostFilesPlugin::addToGUI()
{
    switch (m_pos) {
    case SEPARATE_ACTIVITY:
        getGUI()->addActivity(m_view);
        break;
    case DOCKABLE_WIDGET: {
        KMainWindow *mwnd = getGUI()->getMainWindow();
        m_dock = new QDockWidget(mwnd);
        m_dock->setWidget(m_view);
        m_dock->setObjectName(QStringLiteral("ScanForLostFilesDockWidget"));
        mwnd->addDockWidget(Qt::BottomDockWidgetArea, m_dock);
        break;
    }
    case TORRENT_ACTIVITY:
        getGUI()->getTorrentActivity()->addToolWidget(m_view, m_view->name(), m_view->icon(), m_view->toolTip());
        break;
    }
}

void ScanForLostFilesPlugin::removeFromGUI()
{
    switch (m_pos) {
    case SEPARATE_ACTIVITY:
        getGUI()->removeActivity(m_view);
        break;
    case TORRENT_ACTIVITY:
        getGUI()->getTorrentActivity()->removeToolWidget(m_view);
        break;
    case DOCKABLE_WIDGET: {
        KMainWindow *mwnd = getGUI()->getMainWindow();
        mwnd->removeDockWidget(m_dock);
        m_dock->setWidget(nullptr);
        m_view->setParent(nullptr);
        delete m_dock;
        m_dock = nullptr;
        break;
    }
    }
}
}

#include "scanforlostfilesplugin.moc"
