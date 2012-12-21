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
#ifndef KTWEBCONTENTGENERATOR_H
#define KTWEBCONTENTGENERATOR_H

#include <QString>
#include <QHttpRequestHeader>


namespace kt
{
    class HttpServer;
    class HttpClientHandler;

    /**
        Base class for special pages which generate content or HTML
    */
    class WebContentGenerator
    {
    public:
        enum Permissions
        {
            PUBLIC,
            LOGIN_REQUIRED
        };

        /**
         * Constructor
         * @param path Path of the content e.g. /actions/foobar
         */
        WebContentGenerator(HttpServer* server, const QString& path, Permissions per);
        virtual ~WebContentGenerator();

        /// Get the path
        QString getPath() const {return path;}

        /// Get the permissions (i.e. login required or not)
        Permissions getPermissions() const {return permissions;}

        /**
         * Generate HTML or XML on HTTP a get request
         * @param hdlr Client handler
         * @param hdr HTTP Header of request
         */
        virtual void get(HttpClientHandler* hdlr, const QHttpRequestHeader& hdr) = 0;

        /**
         * Generate HTML or XML on HTTP a get request
         * @param hdlr Client handler
         * @param hdr HTTP Header of request
         * @param data Data of request
         */
        virtual void post(HttpClientHandler* hdlr, const QHttpRequestHeader& hdr, const QByteArray& data) = 0;
    protected:
        HttpServer* server;
        QString path;
        Permissions permissions;
    };

}

#endif
