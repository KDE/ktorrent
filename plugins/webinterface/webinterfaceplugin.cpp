/***************************************************************************
*   Copyright (C) 2006 by Diego R. Brogna                                 *
*   dierbro@gmail.com                                                     *
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

#include <KGenericFactory>
#include <KGlobal>

#include <util/log.h>
#include <util/logsystemmanager.h>
#include <interfaces/coreinterface.h>
#include <interfaces/guiinterface.h>
#include <interfaces/torrentinterface.h>
#include <torrent/globals.h>
#include <net/portlist.h>
#include "webinterfaceprefwidget.h"
#include "webinterfaceplugin.h"
#include "httpserver.h"
#include "webinterfacepluginsettings.h"


K_EXPORT_COMPONENT_FACTORY(ktwebinterfaceplugin, KGenericFactory<kt::WebInterfacePlugin>("ktwebinterfaceplugin"))

using namespace bt;
namespace kt
{
    WebInterfacePlugin::WebInterfacePlugin(QObject* parent, const QStringList& args) : Plugin(parent)
    {
        Q_UNUSED(args);
        http_server = nullptr;
        pref = nullptr;
    }

    WebInterfacePlugin::~WebInterfacePlugin()
    {

    }

    void WebInterfacePlugin::load()
    {
        LogSystemManager::instance().registerSystem(i18n("Web Interface"), SYS_WEB);
        initServer();

        pref = new WebInterfacePrefWidget(nullptr);
        getGUI()->addPrefPage(pref);
        connect(getCore(), SIGNAL(settingsChanged()), this, SLOT(preferencesUpdated()));
    }

    void WebInterfacePlugin::unload()
    {
        LogSystemManager::instance().unregisterSystem(i18n("Web Interface"));
        if (http_server)
        {
            bt::Globals::instance().getPortList().removePort(http_server->getPort(), net::TCP);
            delete http_server;
            http_server = nullptr;
        }

        getGUI()->removePrefPage(pref);
        delete pref;
        pref = nullptr;
        disconnect(getCore(), SIGNAL(settingsChanged()), this, SLOT(preferencesUpdated()));
    }

    void WebInterfacePlugin::initServer()
    {
        bt::Uint16 port = WebInterfacePluginSettings::port();
        bt::Uint16 i = 0;

        while (i < 10)
        {
            http_server = new HttpServer(getCore(), port + i);
            if (!http_server->isOK())
            {
                delete http_server;
                http_server = nullptr;
            }
            else
                break;
            i++;
        }

        if (http_server)
        {
            if (WebInterfacePluginSettings::forward())
                bt::Globals::instance().getPortList().addNewPort(http_server->getPort(), net::TCP, true);
            Out(SYS_WEB | LOG_ALL) << "Web server listen on port " << http_server->getPort() << endl;
        }
        else
        {
            Out(SYS_WEB | LOG_ALL) << "Cannot bind to port " << port << " or the 10 following ports. WebInterface plugin cannot be loaded." << endl;
            return;
        }
    }

    void WebInterfacePlugin::preferencesUpdated()
    {
        if (http_server && http_server->getPort() != WebInterfacePluginSettings::port())
        {
            //stop and delete http server
            bt::Globals::instance().getPortList().removePort(http_server->getPort(), net::TCP);
            delete http_server;
            http_server = nullptr;
            // reinitialize server
            initServer();
        }
    }

    bool WebInterfacePlugin::versionCheck(const QString& version) const
    {
        return version == KT_VERSION_MACRO;
    }
}

#include "webinterfaceplugin.moc"
