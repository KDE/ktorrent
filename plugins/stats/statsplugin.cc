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

#include "statsplugin.h"

K_EXPORT_COMPONENT_FACTORY(ktstatsplugin, KGenericFactory<kt::StatsPlugin>("ktstatsplugin"))

namespace kt
{

StatsPlugin::StatsPlugin(QObject* parent, const char* qt_name, const QStringList& args):
		Plugin(parent, qt_name, args, "Statistics", i18n("Statistics"),"Krzysztof Kundzicz", "athantor@gmail.com", i18n("Shows transfers statistics"),"ktimemon"), pmUiSpd(0), pmUiCon(0), pmPrefsUi(0), pmUpdTmr(0)
{ 
	mUpAvg = std::make_pair(0.0, 0.0);
	mDownAvg = std::make_pair(0.0, 0.0);
	mLeechAvg = std::make_pair(0, 0);
	mRunningLeechAvg = std::make_pair(0, 0);
	mSeedAvg = std::make_pair(0, 0);
	mRunningSeedAvg = std::make_pair(0, 0);
}

StatsPlugin::~StatsPlugin()
{
}

void StatsPlugin::load()
{
	
	mUpdCtr = 1;
	mPeerSpdUpdCtr = 1;
	
	pmUiSpd = new StatsSpd(dynamic_cast<QWidget *>(parent()));
	pmUiCon = new StatsCon(dynamic_cast<QWidget *>(parent()));
	pmPrefsUi = new StatsPluginPrefs();
	pmUpdTmr = new QTimer(this);
	
	connect(pmUpdTmr, SIGNAL(timeout () ), this, SLOT(UpdateData()));
	connect(pmPrefsUi, SIGNAL(Applied()), this, SLOT(RestartTimer()));
	connect(pmPrefsUi, SIGNAL(Applied()), this, SLOT(TogglePeersSpdCht()));
	connect(pmPrefsUi, SIGNAL(Applied()), this, SLOT(ToggleLchInSwmDrawing()));
	connect(pmPrefsUi, SIGNAL(Applied()), this, SLOT(ToggleSdrInSwmDrawing()));
	connect(pmPrefsUi, SIGNAL(Applied()), this, SLOT(ChangeMsmtsCounts()));
	connect(pmPrefsUi, SIGNAL(Applied()), this, SLOT(ChangeMaxMode()));
	
	TogglePeersSpdCht();
	ChangeMaxMode();
	
	pmUpdTmr -> start(StatsPluginSettings::gatherDataEveryMs());

	getGUI() -> addToolWidget(pmUiSpd,"ktimemon" ,i18n("Speed statistics"), GUIInterface::DOCK_BOTTOM);
	getGUI() -> addToolWidget(pmUiCon,"ktimemon" ,i18n("Connection statistics"), GUIInterface::DOCK_BOTTOM);
	getGUI() -> addPrefPage (pmPrefsUi);
	
}

void StatsPlugin::unload()
{
	getGUI() -> removeToolWidget(pmUiSpd);
	getGUI() -> removeToolWidget(pmUiCon);
	getGUI() -> removePrefPage(pmPrefsUi);
	
	disconnect(pmUpdTmr, SIGNAL(timeout()), this, SLOT(UpdateData()));
	disconnect(pmPrefsUi, SIGNAL(Applied()), this, SLOT(RestartTimer()));
	disconnect(pmPrefsUi, SIGNAL(Applied()), this, SLOT(TogglePeersSpdCht()));
	disconnect(pmPrefsUi, SIGNAL(Applied()), this, SLOT(ToggleLchInSwmDrawing()));
	disconnect(pmPrefsUi, SIGNAL(Applied()), this, SLOT(ToggleSdrInSwmDrawing()));
	disconnect(pmPrefsUi, SIGNAL(Applied()), this, SLOT(ChangeMsmtsCounts()));
	disconnect(pmPrefsUi, SIGNAL(Applied()), this, SLOT(ChangeMaxMode()));
	
	delete pmUiSpd;
	delete pmUiCon;
	delete pmPrefsUi;
	delete pmUpdTmr;
}

bool StatsPlugin::versionCheck(const QString& rVer) const
{
	return rVer == KT_VERSION_MACRO;
}

void StatsPlugin::guiUpdate()
{
	if(mUpdCtr >= StatsPluginSettings::updateChartsEveryGuiUpdates())
	{
		pmUiSpd -> UpdateCharts();
		pmUiCon -> UpdateCharts();
		mUpdCtr = 1;
	
	} else {
		mUpdCtr++;
	}	
}

void StatsPlugin::UpdateData()
{	
	uint32_t lcon = 0;
	uint32_t lswa = 0;
	uint32_t scon = 0;
	uint32_t sswa  = 0;
	uint32_t rlcon = 0;
	uint32_t rlswa = 0;
	uint32_t rscon = 0;
	uint32_t rsswa = 0;
	
	uint32_t ld = 0;
	uint32_t lu = 0;
	uint32_t sd = 0;
	
	//---------------------------------------
	
	mDownAvg.first += getCore() -> getStats() . download_speed;
	mDownAvg.second++;
	
	mUpAvg.first += getCore() -> getStats() . upload_speed;
	mUpAvg.second++;
	
	pmUiSpd -> AddDownSpdVal(0, getCore() -> getStats() . download_speed / 1024.0);
 	pmUiSpd -> AddUpSpdVal(0, getCore() -> getStats() . upload_speed / 1024.0);
 	
	pmUiSpd -> AddDownSpdVal(1, (mDownAvg.first / mDownAvg.second) / 1024.0 );
	pmUiSpd -> AddUpSpdVal(1, (mUpAvg.first / mUpAvg.second) / 1024.0 );
	
	pmUiSpd -> AddDownSpdVal(2, getCore() -> getMaxDownloadSpeed () );
	pmUiSpd -> AddUpSpdVal(2, getCore() -> getMaxUploadSpeed ());
	
// 	if(getGUI()-> getCurrentTorrent())
// 	{
// 		pmUi -> AddDownSpdVal(3, getGUI()-> getCurrentTorrent() -> getStats() . download_rate / 1024.0);
//  		pmUi -> AddUpSpdVal(3, getGUI()-> getCurrentTorrent() -> getStats() . upload_rate / 1024.0);
//  	} else {
//  		pmUi -> AddDownSpdVal(3, 0.0);
//  		pmUi -> AddUpSpdVal(3, 0.0);
//  	}
	
	// ------
	
	bt::QueueManager::iterator tor = getCore() -> getQueueManager () -> begin();
	
	while(tor != getCore() -> getQueueManager () -> end())
	{	
		lcon += (*tor) -> getStats().leechers_connected_to;
		lswa += (*tor) -> getStats().leechers_total;
		scon += (*tor) -> getStats().seeders_connected_to;
		sswa += (*tor) -> getStats().seeders_total;
		
		mLeechAvg.first += lcon;
		mLeechAvg.second += lswa;
		mSeedAvg.first += scon;
		mSeedAvg.second += sswa;
		
		if(StatsPluginSettings::peersSpeed() && ( mPeerSpdUpdCtr >= StatsPluginSettings::peersSpeedDataIval() ) )
		{
			bt::TorrentControl * tc = dynamic_cast<bt::TorrentControl *>( *tor );
			const bt::PeerManager * pm = tc->getPeerMgr();
			if(tc && pm)
			{
				for(bt::PeerManager::CItr it = pm -> beginPeerList(); it != pm -> endPeerList (); ++it)
				{
					if(it && (*it) )
					{	
						if(!(*it) -> isSeeder())
						{
							ld += (*it) -> getDownloadRate();
							lu += (*it) -> getUploadRate();
						} else {
							sd += (*it) -> getDownloadRate();
						}
					}
				}
			}
		}
	

		if( (*tor) -> getStats().started)
		{
		
			rlcon += (*tor) -> getStats().leechers_connected_to;
			rlswa += (*tor) -> getStats().leechers_total;
			rscon += (*tor) -> getStats().seeders_connected_to;
			rsswa += (*tor) -> getStats().seeders_total;
		
			mRunningLeechAvg.first += rlcon;
			mRunningLeechAvg.second += rlswa;
			mRunningSeedAvg.first += rscon;
			mRunningSeedAvg.second += rsswa;	
		}
		
		tor++;
	}
		
	// ------
	
	if(StatsPluginSettings::peersSpeed() )
	{
		if( mPeerSpdUpdCtr >= StatsPluginSettings::peersSpeedDataIval() ) 
		{
			pmUiSpd -> AddPeersSpdVal(0, (ld / (lcon * 1.0)) /  1024.0);
			pmUiSpd -> AddPeersSpdVal(1, (lu / (lcon * 1.0)) /  1024.0);
			pmUiSpd -> AddPeersSpdVal(2, (sd / (lswa * 1.0)) /  1024.0);
			pmUiSpd -> AddPeersSpdVal(3, ld / 1024.0);
			pmUiSpd -> AddPeersSpdVal(4, sd / 1024.0);
			
			mPeerSpdUpdCtr = 1;
		} else {
			mPeerSpdUpdCtr++;
		}
	} 
	
	pmUiCon -> AddPeersConVal(0, lcon);
	if(StatsPluginSettings::drawLeechersInSwarms())
	{
		pmUiCon -> AddPeersConVal(1, lswa);
	}
	pmUiCon -> AddPeersConVal(2, scon);
	if(StatsPluginSettings::drawSeedersInSwarms())
	{
		pmUiCon -> AddPeersConVal(3, sswa);
	}
	
	double cnt = getCore() -> getQueueManager() -> count() * 1.0;
	double rcnt = getCore() -> getQueueManager() -> getNumRunning() * 1.0;
	
	pmUiCon -> AddPeersConVal(4, lcon /  cnt );
	pmUiCon -> AddPeersConVal(5, scon /  cnt );
	pmUiCon -> AddPeersConVal(6, lcon / rcnt);
	pmUiCon -> AddPeersConVal(7, scon / rcnt );
	
	// -----
	
	if( bt::Globals::instance().getDHT().isRunning() )
	{
		pmUiCon -> AddDHTVal(0, bt::Globals::instance().getDHT(). getStats().num_peers);
		pmUiCon -> AddDHTVal(1, bt::Globals::instance().getDHT(). getStats().num_tasks);
	}
}

void StatsPlugin::RestartTimer()
{
	if( (!pmUpdTmr) || (!pmUpdTmr -> isActive()))
	{
		return;
	}
	
	pmUpdTmr -> stop();
	pmUpdTmr -> start(StatsPluginSettings::gatherDataEveryMs());
}

void StatsPlugin::TogglePeersSpdCht()
{
	if(StatsPluginSettings::peersSpeed())
	{
		if(pmUiSpd -> PeersSpdGbw -> isHidden())
		{
			pmUiSpd -> PeersSpdGbw -> setHidden(false);
		}
	} else {
		if(!pmUiSpd -> PeersSpdGbw -> isHidden())
		{
			pmUiSpd -> PeersSpdGbw -> setHidden(true);
		}
	}
}

void StatsPlugin::ToggleLchInSwmDrawing()
{
	if(!StatsPluginSettings::drawLeechersInSwarms())
	{
		pmUiCon -> ZeroPeersConn(1);
	}
}

void StatsPlugin::ToggleSdrInSwmDrawing()
{
	if(!StatsPluginSettings::drawSeedersInSwarms())
	{
		pmUiCon -> ZeroPeersConn(3);
	}
}

void StatsPlugin::ChangeMsmtsCounts()
{
	pmUiSpd -> ChangeDownMsmtCnt(StatsPluginSettings::downloadMeasurements());
	pmUiSpd -> ChangePrsSpdMsmtCnt(StatsPluginSettings::peersSpeedMeasurements());
	pmUiSpd -> ChangeUpMsmtCnt(StatsPluginSettings::uploadMeasurements());
	pmUiCon -> ChangeConnMsmtCnt(StatsPluginSettings::connectionsMeasurements());
	pmUiCon -> ChangeDHTMsmtCnt(StatsPluginSettings::dHTMeasurements());
}

void StatsPlugin::ChangeMaxMode()
{
	if(StatsPluginSettings::maxSpdMode() == 0)
	{
		pmUiSpd -> ChangeChartsMaxMode(ChartDrawer::MaxModeTop);
		pmUiCon -> ChangeChartsMaxMode(ChartDrawer::MaxModeTop);
		
	} else if (StatsPluginSettings::maxSpdMode() == 1) {
		pmUiSpd -> ChangeChartsMaxMode(ChartDrawer::MaxModeExact);
		pmUiCon -> ChangeChartsMaxMode(ChartDrawer::MaxModeExact);
	}
}

} // NS end

#include "statsplugin.moc"
