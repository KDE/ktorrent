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

#include <SpdTabPage.h>
#include <peer/peer.h>

namespace kt {

SpdTabPage::SpdTabPage(QWidget *p) : PluginPage(p), pmUiSpd(new Ui::SpdWgt), mDlAvg(std::make_pair(0,0)), mUlAvg(std::make_pair(0,0))
{

	if(StatsPluginSettings::widgetType() == 0)
	{
		pmDlChtWgt.reset(new PlainChartDrawer(this));
		pmPeersChtWgt.reset(new PlainChartDrawer(this));
		pmUlChtWgt.reset(new PlainChartDrawer(this));
		
		connect( dynamic_cast<PlainChartDrawer *>(pmDlChtWgt.get()), SIGNAL(Zeroed(ChartDrawer *)), this, SLOT(ResetAvg(ChartDrawer *)));
		connect( dynamic_cast<PlainChartDrawer *>(pmUlChtWgt.get()), SIGNAL(Zeroed(ChartDrawer *)), this, SLOT(ResetAvg(ChartDrawer *)));
		
	} else if(StatsPluginSettings::widgetType() == 1) {
		pmDlChtWgt.reset(new KPlotWgtDrawer(this));
		pmPeersChtWgt.reset(new KPlotWgtDrawer(this));
		pmUlChtWgt.reset(new KPlotWgtDrawer(this));
		
		connect( dynamic_cast<KPlotWgtDrawer *>(pmDlChtWgt.get()), SIGNAL(Zeroed(ChartDrawer *)), this, SLOT(ResetAvg(ChartDrawer *)));
		connect( dynamic_cast<KPlotWgtDrawer *>(pmUlChtWgt.get()), SIGNAL(Zeroed(ChartDrawer *)), this, SLOT(ResetAvg(ChartDrawer *)));
	}
	
	

	SetupUi();
}

SpdTabPage::~SpdTabPage()
{
}

void SpdTabPage::SetupUi()
{
	pmUiSpd -> setupUi(this);
	
	pmUiSpd -> DlSpdGbw -> layout() -> addWidget(dynamic_cast<QWidget *>(pmDlChtWgt.get()));
	pmUiSpd -> PeersSpdGbw -> layout() -> addWidget(dynamic_cast<QWidget *>(pmPeersChtWgt.get()));
	pmUiSpd -> UlSpdGbw -> layout() -> addWidget(dynamic_cast<QWidget *>(pmUlChtWgt.get()));
	
	pmDlChtWgt -> AddDataSet(ChartDrawerData(i18n("Current speed"), QPen(StatsPluginSettings::dlSpdColor()), true));
	pmUlChtWgt -> AddDataSet(ChartDrawerData(i18n("Current speed"), QPen(StatsPluginSettings::ulSpdColor()), true));
	
	pmDlChtWgt -> AddDataSet(ChartDrawerData(i18n("Average speed"), QPen(StatsPluginSettings::dlAvgColor()), true));
	pmUlChtWgt -> AddDataSet(ChartDrawerData(i18n("Average speed"), QPen(StatsPluginSettings::ulAvgColor()), true));
	
	pmDlChtWgt -> AddDataSet(ChartDrawerData(i18n("Speed limit"), QPen(StatsPluginSettings::dlLimitColor()), true));
	pmUlChtWgt -> AddDataSet(ChartDrawerData(i18n("Speed limit"), QPen(StatsPluginSettings::ulLimitColor()), true));
	
	pmPeersChtWgt -> AddDataSet(ChartDrawerData(i18n("Average from leechers"), QPen(StatsPluginSettings::prAvgFromLColor()), true));
	pmPeersChtWgt -> AddDataSet(ChartDrawerData(i18n("Average to leechers"), QPen(StatsPluginSettings::prAvgToLColor()), true));
	pmPeersChtWgt -> AddDataSet(ChartDrawerData(i18n("Average from seeds"), QPen(StatsPluginSettings::prAvgFromSColor()), true));
	pmPeersChtWgt -> AddDataSet(ChartDrawerData(i18n("From leechers"), QPen(StatsPluginSettings::prFromLColor()), true));
	pmPeersChtWgt -> AddDataSet(ChartDrawerData(i18n("From seeds"), QPen(StatsPluginSettings::prFromSColor()), true));
	
	ApplySettings();
}

void SpdTabPage::ApplySettings()
{

	pmDlChtWgt -> SetPen(0, QPen(StatsPluginSettings::dlSpdColor()));
	pmUlChtWgt -> SetPen(0, QPen(StatsPluginSettings::ulSpdColor()));
	
	pmDlChtWgt -> SetPen(1, QPen(StatsPluginSettings::dlAvgColor()));
	pmUlChtWgt -> SetPen(1, QPen(StatsPluginSettings::ulAvgColor()));
	
	pmDlChtWgt -> SetPen(2, QPen(StatsPluginSettings::dlLimitColor()));
	pmUlChtWgt -> SetPen(2, QPen(StatsPluginSettings::ulLimitColor()));
	
	pmPeersChtWgt -> SetPen(0, QPen(StatsPluginSettings::prAvgFromLColor()));
	pmPeersChtWgt -> SetPen(1, QPen(StatsPluginSettings::prAvgToLColor()));
	pmPeersChtWgt -> SetPen(2, QPen(StatsPluginSettings::prAvgFromSColor()));
	pmPeersChtWgt -> SetPen(3, QPen(StatsPluginSettings::prFromLColor()));
	pmPeersChtWgt -> SetPen(4, QPen(StatsPluginSettings::prFromSColor()));

	pmDlChtWgt -> SetXMax(StatsPluginSettings::dlSpdSamples());
	pmPeersChtWgt -> SetXMax(StatsPluginSettings::peersSpdSamples());
	pmUlChtWgt -> SetXMax(StatsPluginSettings::ulSpdSamples());
	
	pmDlChtWgt -> SetMaxMode(static_cast<ChartDrawer::MaxMode>(StatsPluginSettings::maxMode()));
	pmPeersChtWgt -> SetMaxMode(static_cast<ChartDrawer::MaxMode>(StatsPluginSettings::maxMode()));
	pmUlChtWgt -> SetMaxMode(static_cast<ChartDrawer::MaxMode>(StatsPluginSettings::maxMode()));
	
	// ---
	
	pmDlChtWgt -> EnableAntiAlias(StatsPluginSettings::antiAliasing());
	pmPeersChtWgt -> EnableAntiAlias(StatsPluginSettings::antiAliasing());
	pmUlChtWgt -> EnableAntiAlias(StatsPluginSettings::antiAliasing());
	
	pmDlChtWgt -> EnableBgdGrid(StatsPluginSettings::drawBgdGrid());
	pmPeersChtWgt -> EnableBgdGrid(StatsPluginSettings::drawBgdGrid());
	pmUlChtWgt -> EnableBgdGrid(StatsPluginSettings::drawBgdGrid());
	
}

void SpdTabPage::UpdateAllCharts()
{
	pmDlChtWgt -> update();
	pmPeersChtWgt -> update();
	pmUlChtWgt -> update();
}

void SpdTabPage::GatherDlSpeed(Plugin * pPlug)
{
	uint spd = pPlug -> getCore() -> getStats().download_speed;
	mDlAvg.first += spd;
	mDlAvg.second++;

	pmDlChtWgt -> AddValue(0,  spd / 1024.0 );
	pmDlChtWgt -> AddValue(1,  (mDlAvg.first / mDlAvg.second) / 1024.0);
	pmDlChtWgt -> AddValue(2, Settings::maxDownloadRate());
}

void SpdTabPage::GatherPeersSpeed(Plugin * pPlug)
{
	kt::QueueManager * qm_iface = pPlug -> getCore() -> getQueueManager();
	
	if(qm_iface == 0)
	{
		return;
	}
	
	uint_least64_t l_up_spd, l_dn_spd, s_dn_spd;
	uint_least32_t l_cnt, s_cnt;
	
	l_up_spd = l_dn_spd = s_dn_spd = l_cnt = s_cnt = 0;
	
	for(QList< bt::TorrentInterface *>::iterator it = qm_iface -> begin(); it != qm_iface -> end(); it++)
	{	
		bt::TorrentControl * tctl = dynamic_cast<bt::TorrentControl *>(*it);
		if(!tctl)
		{
			continue;
		}
		
		const bt::PeerManager * p_mgr = tctl -> getPeerMgr();
		
		QList<bt::Peer*> ppl = p_mgr->getPeers();
		foreach (bt::Peer* peer,ppl)
		{
			const bt::PeerInterface::Stats p_stats = peer -> getStats();
			
			if(p_stats.perc_of_file >= 100)
			{
				s_dn_spd += p_stats.download_rate;
				s_cnt++;
				
			} else {
				l_dn_spd += p_stats.download_rate;
				l_up_spd += p_stats.upload_rate;
				l_cnt++;
			}
		}
	}
	
	if(!l_cnt)
	{
		pmPeersChtWgt -> AddValue(0, 0);
		pmPeersChtWgt -> AddValue(1, 0);
		pmPeersChtWgt -> AddValue(3, 0);
	} else {
		pmPeersChtWgt -> AddValue(0, (static_cast<double>(l_dn_spd) / static_cast<double>(l_cnt)) / 1024.0 );
		pmPeersChtWgt -> AddValue(1, (static_cast<double>(l_up_spd) / static_cast<double>(l_cnt)) / 1024.0);
		pmPeersChtWgt -> AddValue(3, static_cast<double>(l_dn_spd) / 1024.0);
	}
	
	if(!s_cnt)
	{
		pmPeersChtWgt -> AddValue(2, 0);
		pmPeersChtWgt -> AddValue(4, 0);
	} else {
		pmPeersChtWgt -> AddValue(2, (static_cast<double>(s_dn_spd) / static_cast<double>(s_cnt)) / 1024.0 );
		pmPeersChtWgt -> AddValue(4, static_cast<double>(s_dn_spd) / 1024.0);
	}
	
	
}

void SpdTabPage::GatherUlSpeed(Plugin * pPlug)
{
	uint spd = pPlug -> getCore() -> getStats().upload_speed;
	mUlAvg.first += spd;
	mUlAvg.second++;
	
	pmUlChtWgt -> AddValue(0, spd / 1024.0);
	pmUlChtWgt -> AddValue(1,  (mUlAvg.first / mUlAvg.second) / 1024.0 );
	pmUlChtWgt -> AddValue(2, Settings::maxUploadRate());
}

void SpdTabPage::GatherData(Plugin * pPlug)
{
	GatherDlSpeed(pPlug);
	GatherPeersSpeed(pPlug);
	GatherUlSpeed(pPlug);
}

void SpdTabPage::ResetAvg(ChartDrawer * c)
{
	if(!c)
	{
		return;
	}
	else if(c == pmDlChtWgt.get())
	{
		mDlAvg = std::make_pair(0,0);
	}
	else if(c == pmUlChtWgt.get())
	{
		mUlAvg = std::make_pair(0,0);
	}
	else 
	{
		qDebug("Got unreckognized widget!");
	}
}

} //ns e
