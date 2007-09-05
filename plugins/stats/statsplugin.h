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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef StatsPlugin_H_
#define StatsPlugin_H_

#include <kgenericfactory.h>

#include <qwidget.h>
#include <qtimer.h>

#include <interfaces/plugin.h>
#include <interfaces/guiinterface.h>
#include <interfaces/coreinterface.h>
#include <torrent/globals.h>
#include <kademlia/dhtbase.h>
#include <torrent/queuemanager.h>
#include <torrent/torrentcontrol.h>
#include <torrent/peermanager.h>
#include <torrent/peer.h>

#include "StatsSpd.h"
#include "StatsCon.h"
#include "StatsPluginPrefs.h"
#include "statspluginsettings.h"
#include <map> // std::pair

namespace kt {

/**
\brief Statistics plugin
\author Krzysztof Kundzicz <athantor@gmail.com>
\version 200705191548
*/
class StatsPlugin : public Plugin
{
	Q_OBJECT

	private:
		///Speed UI of the plugin
		StatsSpd * pmUiSpd;
		///Connections UI of the plugin
		StatsCon * pmUiCon;
		///UI of the pref page
		StatsPluginPrefs * pmPrefsUi;
		/**
		\brief Average upload speed data
		
		\li \c first: Total speed
		\li \c second: Measurements count
		*/
		std::pair<long double, long double> mUpAvg;
		/**
		\brief Average download speed data
		
		\li \c first: Total speed
		\li \c second: Measurements count
		*/
		std::pair<long double, long double> mDownAvg;
		/**
		\brief Leechers stats
		
		\li \c first: connected
		\li \c second: swarm
		*/
		std::pair<uint32_t, uint32_t> mLeechAvg;
		/**
		\brief Leechers stats on running torrents
		
		\li \c first: connected
		\li \c second: swarm
		*/
		std::pair<uint32_t, uint32_t> mRunningLeechAvg;
		/**
		\brief Seeders stats
		
		\li \c first: connected
		\li \c second: swarm
		*/
		std::pair<uint32_t, uint32_t> mSeedAvg;
		/**
		\brief Seeders stats on running torrents
		
		\li \c first: connected
		\li \c second: swarm
		*/
		std::pair<uint32_t, uint32_t> mRunningSeedAvg;
		
		///Data update timer
		QTimer * pmUpdTmr;
		
		///Update ctr
		uint32_t mUpdCtr;
		uint32_t mPeerSpdUpdCtr;
	
	private slots:
		///Updates stat data
		void UpdateData();
		/**
		\brief Restarts timer
		
		Restarts timer when the interval of data gathering has been changed
		*/
		void RestartTimer();
		///Toggles peers speed chart
		void TogglePeersSpdCht();
		///Toggles drawing of total leechers in swarms
		void ToggleLchInSwmDrawing();
		///Toggles drawing of total seeders in swarms
		void ToggleSdrInSwmDrawing();
		///Changes measurements counts
		void ChangeMsmtsCounts();
		///Changes OY max mode
		void ChangeMaxMode();

	public:
		/**
		\brief Constructor
		\param parent Parent
		\param qt_name 
		\param args
		*/
		StatsPlugin(QObject* parent, const char* qt_name, const QStringList& args);
		///Destructor
		virtual ~StatsPlugin();
		
		virtual void load();
		virtual void unload();
		virtual bool versionCheck(const QString&) const;
		virtual void guiUpdate();
};

}

#endif

