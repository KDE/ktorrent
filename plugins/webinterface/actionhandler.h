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

#ifndef KTACTIONHANDLER_H
#define KTACTIONHANDLER_H

#include <webcontentgenerator.h>

namespace kt
{

    /**
        Handles actions coming from the client
    */
    class ActionHandler : public WebContentGenerator
    {
    public:
        ActionHandler(CoreInterface* core, HttpServer* server);
        virtual ~ActionHandler();

        virtual void get(HttpClientHandler* hdlr, const QHttpRequestHeader& hdr);
        virtual void post(HttpClientHandler* hdlr, const QHttpRequestHeader& hdr, const QByteArray& data);
    private:
        bool doCommand(const QString& cmd, const QString& arg);
        bool dht(const QString& arg);
        bool encryption(const QString& arg);
        bool file(const QString& cmd, const QString& arg);

    private:
        CoreInterface* core;
    };

}

#endif
