/*
    SPDX-FileCopyrightText: 2008 Joris Guisson <joris.guisson@gmail.com>
    SPDX-FileCopyrightText: 2008 Ivan Vasic <ivasic@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mediaplayerplugin.h"

#include <KLocalizedString>
#include <KPluginFactory>

#include "mediaplayeractivity.h"
#include <interfaces/guiinterface.h>
#include <util/log.h>
#include <util/logsystemmanager.h>

K_PLUGIN_FACTORY_WITH_JSON(ktorrent_mediaplayer, "ktorrent_mediaplayer.json", registerPlugin<kt::MediaPlayerPlugin>();)

using namespace bt;

namespace kt
{
MediaPlayerPlugin::MediaPlayerPlugin(QObject *parent, const QVariantList &args)
    : Plugin(parent)
{
    Q_UNUSED(args);
}

MediaPlayerPlugin::~MediaPlayerPlugin()
{
}

void MediaPlayerPlugin::load()
{
    LogSystemManager::instance().registerSystem(i18n("Media Player"), SYS_MPL);
    CoreInterface *core = getCore();
    act = new MediaPlayerActivity(core, actionCollection(), nullptr);
    getGUI()->addActivity(act);
    setXMLFile(QStringLiteral("ktorrent_mediaplayerui.rc"));
    act->enableActions(0);
    act->loadState(KSharedConfig::openConfig());
}

void MediaPlayerPlugin::unload()
{
    LogSystemManager::instance().unregisterSystem(i18n("Media Player"));
    act->saveState(KSharedConfig::openConfig());
    act->setVideoFullScreen(false);
    getGUI()->removeActivity(act);
    delete act;
    act = nullptr;
}

bool MediaPlayerPlugin::versionCheck(const QString &version) const
{
    return version == QStringLiteral(VERSION);
}

}

#include "mediaplayerplugin.moc"
