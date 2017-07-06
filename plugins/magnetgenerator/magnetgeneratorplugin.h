/***************************************************************************
*   Copyright (C) 2010 by Jonas Lundqvist                                 *
*   jonas@gannon.se                                                       *
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

#ifndef KT_MAGNETGENERATORPLUGIN_H
#define KT_MAGNETGENERATORPLUGIN_H

#include <interfaces/plugin.h>
#include <interfaces/torrentactivityinterface.h>

namespace kt
{
    class MagnetGeneratorPrefWidget;

    class MagnetGeneratorPlugin : public Plugin, public ViewListener
    {
        Q_OBJECT
    public:
        MagnetGeneratorPlugin(QObject* parent, const QVariantList& args);
        ~MagnetGeneratorPlugin();

        void load() override;
        void unload() override;
        bool versionCheck(const QString& version) const override;
        QString parentPart() const override {return QStringLiteral("torrentactivity");}
        void currentTorrentChanged(bt::TorrentInterface* tc) override;

    private slots:
        void generateMagnet();

    private:
        MagnetGeneratorPrefWidget* pref;
        QAction * generate_magnet_action;
        void addToClipboard(QString uri);
        void showPopup();
    };

}

#endif // KT_MAGNETGENERATORPLUGIN_H
