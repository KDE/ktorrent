/***************************************************************************
 *   Copyright © 2007 by Krzysztof Kundzicz                                *
 *   athantor@gmail.com                                                    *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/

#ifndef StatsPlugin_H_
#define StatsPlugin_H_

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>

#include <interfaces/plugin.h>
#include <interfaces/guiinterface.h>

#include <memory>

#include <SpdTabPage.h>
#include <ConnsTabPage.h>
#include <SettingsPage.h>
#include <DisplaySettingsPage.h>
#include <statspluginsettings.h>

namespace kt
{

    /** \brief Statistics plugin
    \author Krzysztof Kundzicz <athantor@gmail.com>
    \version 1.1
    */

    class StatsPlugin : public Plugin
    {
        Q_OBJECT
    public:
        /** \brief Constructor
        \param p Parent
        */
        StatsPlugin(QObject* p, const QVariantList&);
        ///Destructor
        ~StatsPlugin();

        void load() override;
        void unload() override;
        bool versionCheck(const QString& version) const override;
        void guiUpdate() override;

    public slots:
        ///Gather data
        void gatherData();
        ///Settings has been changed
        void settingsChanged();

    private:
        ///Speeds tab
        SpdTabPage* pmUiSpd;
        ///Connections tab
        ConnsTabPage* pmUiConns;
        ///Settings Page
        SettingsPage* pmUiSett;
        /// Display settings page
        DisplaySettingsPage* pmDispSett;
        ///Timer
        QTimer pmTmr;

        ///Updates counter
        uint32_t mUpdCtr;
    };

} //ns end

#endif
