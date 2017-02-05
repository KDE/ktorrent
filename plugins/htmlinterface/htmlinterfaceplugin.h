/***************************************************************************
 *   Copyright (C) 2008 by Joris Guisson and Ivan Vasic                    *
 *   joris.guisson@gmail.com                                               *
 *   ivasic@gmail.com                                                      *
 *                                                                         *
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

#ifndef KTHTMLINTERFACEPLUGIN_H
#define KTHTMLINTERFACEPLUGIN_H

#include <util/ptrmap.h>
#include <interfaces/torrentactivityinterface.h>
#include <interfaces/plugin.h>
#include <QtGlobal>
#include "webserver.h"


namespace kt
{  
    class HtmlInterfacePlugin: public Plugin, public ViewListener
    {
        Q_OBJECT
    public:
        HtmlInterfacePlugin(QObject* parent, const QVariantList& args);
        ~HtmlInterfacePlugin();

        bool versionCheck(const QString& version) const override;
        void load() override;
        void unload() override;
        void currentTorrentChanged(bt::TorrentInterface* tc) override;
        
        HtmlInterfacePlugin * instance;
//         QString parentPart() const override {return QStringLiteral("torrentactivity");}
// 
//         /// Get the download order manager for a torrent (returns 0 if none exists)
//         HtmlInterfaceManager* manager(bt::TorrentInterface* tc);
// 
//         /// Create a manager for a torrent
//         HtmlInterfaceManager* createManager(bt::TorrentInterface* tc);
// 
//         /// Destroy a manager
//         void destroyManager(bt::TorrentInterface* tc);
// 
//     private slots:
//         void showHtmlInterfaceDialog();
//         void torrentAdded(bt::TorrentInterface* tc);
//         void torrentRemoved(bt::TorrentInterface* tc);
// 
    private:
        QThread* thread;
        WebServer * worker;
//         QAction * download_order_action;
//         bt::PtrMap<bt::TorrentInterface*, HtmlInterfaceManager> managers;
     };

}

#endif
