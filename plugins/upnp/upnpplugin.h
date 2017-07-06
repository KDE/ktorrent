/***************************************************************************
 *   Copyright (C) 2005-2007 by Joris Guisson                              *
 *   joris.guisson@gmail.com                                               *
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

#ifndef KTSEARCHPLUGIN_H
#define KTSEARCHPLUGIN_H

#include <interfaces/plugin.h>

namespace bt
{
    class UPnPMCastSocket;
}

namespace kt
{
    class UPnPWidget;

    /**
    @author Joris Guisson
    */
    class UPnPPlugin : public Plugin
    {
        Q_OBJECT
    public:
        UPnPPlugin(QObject* parent, const QVariantList& args);
        ~UPnPPlugin();

        void load() override;
        void unload() override;
        void shutdown(bt::WaitJob* job) override;
        bool versionCheck(const QString& version) const override;
    private:
        bt::UPnPMCastSocket* sock;
        UPnPWidget* upnp_tab;
    };

}

#endif
