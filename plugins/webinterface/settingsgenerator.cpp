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

#include <QXmlStreamWriter>

#include <util/log.h>
#include <interfaces/coreinterface.h>
#include "httpserver.h"
#include "httpclienthandler.h"
#include "httpresponseheader.h"
#include "settingsgenerator.h"
#include "settings.h"
#include "webinterfacepluginsettings.h"

using namespace bt;

namespace kt
{

    SettingsGenerator::SettingsGenerator(CoreInterface* core, HttpServer* server)
        : WebContentGenerator(server, "/data/settings.xml", LOGIN_REQUIRED), core(core)
    {
    }


    SettingsGenerator::~SettingsGenerator()
    {
    }


    void SettingsGenerator::get(HttpClientHandler* hdlr, const QHttpRequestHeader& hdr)
    {
        Q_UNUSED(hdr);
        HttpResponseHeader rhdr(200);
        server->setDefaultResponseHeaders(rhdr, "text/xml", true);

        QByteArray output_data;
        QXmlStreamWriter out(&output_data);
        out.setAutoFormatting(true);
        out.writeStartDocument();
        out.writeStartElement("settings");
        KConfigSkeletonItem::List items = Settings::self()->items();
        foreach (const KConfigSkeletonItem* item, items)
        {
            out.writeStartElement(item->name());
            out.writeCharacters(item->property().toString());
            out.writeEndElement();
        }

        // special webinterface settings
        out.writeStartElement("webgui_automatic_refresh");
        out.writeCharacters(WebInterfacePluginSettings::automaticRefresh() ? "true" : "false");
        out.writeEndElement();

        out.writeEndElement();
        out.writeEndDocument();
        hdlr->send(rhdr, output_data);
    }

    void SettingsGenerator::post(HttpClientHandler* hdlr, const QHttpRequestHeader& hdr, const QByteArray& data)
    {
        QStringList params = QString(data).split("&");
        foreach (const QString& p, params)
        {
            // p should look like param=value
            QStringList items = p.split("=");
            if (items.count() != 2)
                continue;

            QString cfg_param = items.at(0);
            QString cfg_value = items.at(1);
            KConfigSkeletonItem* item = Settings::self()->findItem(cfg_param);
            if (!item)
            {
                if (cfg_param == "webgui_automatic_refresh")
                {
                    WebInterfacePluginSettings::setAutomaticRefresh(cfg_value == "1");
                    WebInterfacePluginSettings::self()->save();
                }
            }
            else
                item->setProperty(cfg_value);
        }
        core->applySettings();
        Settings::self()->save();
        get(hdlr, hdr);
    }

}
