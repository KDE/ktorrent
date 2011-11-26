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

#include <QWidget>
#include <QString>
#include <QPen>
#include <QList>

#include <klocale.h>

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
#include <stdint.h>

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
	private:
		///Page's UI
		std::auto_ptr<Ui::SpdWgt> pmUiSpd;

		///Dl speeds chart widget
		std::auto_ptr<ChartDrawer> pmDlChtWgt;
		///Peers speeds chart widget
		std::auto_ptr<ChartDrawer> pmPeersChtWgt;
		///Ul speeds chart widget
		std::auto_ptr<ChartDrawer> pmUlChtWgt;

		///Dl average
		avg_t mDlAvg;
		///Ul average
		avg_t mUlAvg;

		void SetupUi();

		/** \brief Gathers dl speeds data
		\param  pP kt::Plugin interface
		*/
		void GatherDlSpeed(Plugin * pP);
		/** \brief Gathers peers speeds data
		\param  pP kt::Plugin interface
		*/
		void GatherPeersSpeed(Plugin * pP);
		/** \brief Gathers Ul speeds data
		\param  pP kt::Plugin interface
		*/
		void GatherUlSpeed(Plugin * pP);

	public:
		/** \brief Constructor
		\param p Parent
		*/
		SpdTabPage(QWidget *p);
		///Destructor
		~SpdTabPage();

	public slots:
		void ApplySettings();
		void UpdateAllCharts();
		void GatherData(Plugin *);
		void ResetAvg(ChartDrawer *);
	};

} //ns end

#endif
