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
#ifndef ConnsTabPage_H_
#define ConnsTabPage_H_

#include <QWidget>
#include <QUuid>

#include <interfaces/plugin.h>
#include <torrent/globals.h>
#include <dht/dhtbase.h>
#include <interfaces/coreinterface.h>
#include <interfaces/torrentinterface.h>
#include <torrent/queuemanager.h>

#include <memory>
#include <cstdint>

#include <ui_Conns.h>

#include <PluginPage.h>
#include <drawer/ChartDrawer.h>
#include <drawer/PlainChartDrawer.h>
#include <drawer/KPlotWgtDrawer.h>
#include <statspluginsettings.h>

namespace kt
{

    /** \brief Connections tab
    \author Krzysztof Kundzicz <athantor@gmail.com>
    */

    class ConnsTabPage : public PluginPage
    {
        Q_OBJECT

    private:
        ///Tab's UI
        Ui::ConnsWgt* pmConnsUi;

        ///Connections chart widget
        ChartDrawer* pmConnsChtWgt;
        ///DHT chart widget
        ChartDrawer* pmDhtChtWgt;

        /** \brief Leechers in swarms dataset UUID

        Used for identification whether this dataset is already shown on the chart and if it needs to be removed or added on settings chage
        */
        const QUuid pmLhrSwnUuid;
        /** \brief Seeds in swarms dataset UUID

        Used for identification whether this dataset is already shown on the chart and if it needs to be removed or added on settings chage
        */
        const QUuid pmSesSwnUuid;

        /** \brief Gathers data about connections
        \param  pPlug kt::Plugin interface
        */
        void GatherConnStats(Plugin* pPlug);
        /** \brief Gathers data about DHT
        */
        void GatherDhtStats();

        void setupUi() override;
    public:
        /** \brief Constructor
        \param  p Parent
        */
        ConnsTabPage(QWidget* p);
        ///Destructor
        ~ConnsTabPage();


    public slots:
        void applySettings() override;
        void updateAllCharts() override;
        void gatherData(Plugin*) override;
        void resetAvg(ChartDrawer*) override;
    };

} // ns end

#endif
