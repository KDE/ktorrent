/***************************************************************************
 *   Copyright (C) 2018 by Emmanuel Eytan                                  *
 *   eje211@gmail.com                                                      *
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

#include <iostream>
#include "webserver.h"

using namespace std;

#include <stdio.h>
#include <string.h>
#include <QString>
#include <QRegExp>
#include <QStringList>

using namespace std;

namespace kt {
    WebServer::WebServer(CoreInterface * core): core(core)
    {
        
    }
    
    WebServer::~WebServer()
    {
    }

    TorrentListGenerator * WebServer::listGenerator;
    
    void WebServer::process()
    {
        listGenerator = new TorrentListGenerator(core);
        struct mg_connection *c;

        mg_mgr_init(&mgr, NULL);
        c = mg_bind(&mgr, "8880", WebServer::httpeventhandler);
        mg_set_protocol_http_websocket(c);
        while (true) {
            mg_mgr_poll(&mgr, 1000);
        }
    }

    void WebServer::httpeventhandler(struct mg_connection * c, int ev, void * ev_data) {
        if (ev == MG_EV_HTTP_REQUEST)
        {
            struct mg_serve_http_opts opts;

            memset(&opts, 0, sizeof(opts));  // Reset all options to defaults
            opts.document_root = "/usr/share/ktorrent/html/";
                      
            struct http_message * message = (struct http_message *) ev_data;

            char * method = new char[message->method.len + 1];
            strncpy(method, message->method.p, message->method.len);
            method[message->method.len] = '\0';

            char * uri = new char[message->uri.len + 1];
            strncpy(uri, message->uri.p, message->uri.len);
            uri[message->uri.len] = '\0';

            if (strcmp(uri, "/ktorrentdata") == 0 && strcmp(method, "GET") == 0)
            {
                char * json = NULL;
                int size;
                listGenerator->get(&json, &size);
                mg_send_head(c, 200, size, "Content-Type: application/json\nAccess-Control-Allow-Origin: *");
                mg_printf(c, "%s", json);
                return;
            }
            if (strcmp(uri, "/ktorrentaction") == 0)
            {
                char * body = new char[message->body.len + 1];
                strncpy(body, message->body.p, message->body.len);
                body[message->body.len] = '\0';

                if (strcmp(method, "POST") == 0) {
                    listGenerator->post(body);
                }
                const char * validation = "{\"status\":\"Request received.\"}";
                int len = strlen(validation);
                mg_send_head(c, 200, len, "Content-Type: application/json\nAccess-Control-Allow-Origin: *\nAccess-Control-Allow-Headers: X-ACCESS_TOKEN, Access-Control-Allow-Origin, Authorization, Origin, x-requested-with, Content-Type, Content-Range, Content-Disposition, Content-Description\nAccess-Control-Allow-Methods: GET,POST,PUT,DELETE,OPTIONS");
                mg_printf(c, "%s", validation);
                return;
            }
            mg_serve_http(c, (struct http_message *) ev_data, opts);
        }
    }
}
