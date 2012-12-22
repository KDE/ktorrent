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
#include <klocale.h>
#include <kgenericfactory.h>
#include <util/log.h>
#include <util/logsystemmanager.h>
#include <interfaces/guiinterface.h>
#include "mediaplayerplugin.h"
#include "mediaplayeractivity.h"


K_EXPORT_COMPONENT_FACTORY(ktmediaplayerplugin, KGenericFactory<kt::MediaPlayerPlugin>("ktmediaplayerplugin"))

using namespace bt;

namespace kt
{

    MediaPlayerPlugin::MediaPlayerPlugin(QObject* parent, const QStringList& args) : Plugin(parent)
    {
        Q_UNUSED(args);
    }


    MediaPlayerPlugin::~MediaPlayerPlugin()
    {
    }

    void MediaPlayerPlugin::load()
    {
        LogSystemManager::instance().registerSystem(i18n("Media Player"), SYS_MPL);
        CoreInterface* core = getCore();
        act = new MediaPlayerActivity(core, actionCollection(), 0);
        getGUI()->addActivity(act);
        setXMLFile("ktmediaplayerpluginui.rc");
        act->enableActions(0);
        act->loadState(KGlobal::config());
    }

    void MediaPlayerPlugin::unload()
    {
        LogSystemManager::instance().unregisterSystem(i18n("Media Player"));
        act->saveState(KGlobal::config());
        act->setVideoFullScreen(false);
        getGUI()->removeActivity(act);
        delete act;
        act = 0;
    }

    bool MediaPlayerPlugin::versionCheck(const QString& version) const
    {
        return version == KT_VERSION_MACRO;
    }


}
