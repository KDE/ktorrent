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

#ifndef KHTMLINTERFACEWEBSERVER_H
#define KHTMLINTERFACEWEBSERVER_H

#include <QtGlobal>
#include <QObject>

#include <interfaces/coreinterface.h>
#include "torrentlistgenerator.h"

#include "mongoose.h"

namespace kt
{
    class WebServer : public QObject
    {
        Q_OBJECT
    public:
        WebServer(CoreInterface * core);
        ~WebServer();
    public slots:
        void process();
        static void httpeventhandler(struct mg_connection * c, int ev, void * ev_data);
    signals:
        void finished();
        void error(QString err);
    private:
        struct mg_mgr mgr;
        CoreInterface * core;
        static TorrentListGenerator * listGenerator;
    };
}

#endif
