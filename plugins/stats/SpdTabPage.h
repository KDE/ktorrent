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

#ifndef SpdTabPage_H_
#define SpdTabPage_H_

#include <QList>
#include <QString>
#include <QPen>
#include <QWidget>

#include <interfaces/plugin.h>
#include <interfaces/coreinterface.h>
#include <torrent/torrentcontrol.h>
#include <interfaces/torrentinterface.h>
#include <torrent/queuemanager.h>
#include <peer/peermanager.h>
#include <peer/peer.h>
#include <interfaces/peerinterface.h>
#include <settings.h>

#include <memory>
#include <cstdint>

#include <PluginPage.h>
#include <drawer/ChartDrawer.h>
#include <drawer/PlainChartDrawer.h>
#include <drawer/KPlotWgtDrawer.h>
#include <drawer/ChartDrawerData.h>
#include <statspluginsettings.h>

#include <ui_Spd.h>

namespace kt
{

    /** \brief Speeds tab
    \author Krzysztof Kundzicz <athantor@gmail.com>
    */

    class SpdTabPage : public PluginPage
    {
        Q_OBJECT
    public:
        /** \brief Constructor
        \param p Parent
        */
        SpdTabPage(QWidget* p);
        ///Destructor
        ~SpdTabPage();

    public slots:
        void applySettings() override;
        void updateAllCharts() override;
        void gatherData(Plugin*) override;
        void resetAvg(ChartDrawer*) override;

    private:
        /** \brief Gathers dl speeds data
         \ param  pP kt::Plugin interfac*e *
         */
        void gatherDownloadSpeed(Plugin* pP);
        /** \brief Gathers peers speeds data
         \ param  pP kt::Plugin interfac*e *
         */
        void gatherPeersSpeed(Plugin* pP);
        /** \brief Gathers Ul speeds data
         \ param  pP kt::Plugin interfac*e *
         */
        void gatherUploadSpeed(Plugin* pP);

        void setupUi() override;

    private:
        ///Page's UI
        Ui::SpdWgt* pmUiSpd;

        ///Dl speeds chart widget
        ChartDrawer* pmDlChtWgt;
        ///Peers speeds chart widget
        ChartDrawer* pmPeersChtWgt;
        ///Ul speeds chart widget
        ChartDrawer* pmUlChtWgt;

        ///Dl average
        avg_t mDlAvg;
        ///Ul average
        avg_t mUlAvg;
    };

} //ns end

#endif
