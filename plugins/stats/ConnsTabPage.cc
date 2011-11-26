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

#include <ConnsTabPage.h>

namespace kt
{

	ConnsTabPage::ConnsTabPage(QWidget * p) : PluginPage(p), pmConnsUi(new Ui::ConnsWgt), pmLhrSwnUuid(new QUuid(QUuid::createUuid())),
			pmSesSwnUuid(new QUuid(QUuid::createUuid()))
	{

		if (StatsPluginSettings::widgetType() == 0)
		{
			pmConnsChtWgt.reset(new PlainChartDrawer(this));
			pmDhtChtWgt.reset(new PlainChartDrawer(this));
		}
		else if (StatsPluginSettings::widgetType() == 1)
		{
			pmConnsChtWgt.reset(new KPlotWgtDrawer(this));
			pmDhtChtWgt.reset(new KPlotWgtDrawer(this));
		}

		SetupUi();
	}

	ConnsTabPage::~ConnsTabPage()
	{
	}


	void ConnsTabPage::SetupUi()
	{
		pmConnsUi->setupUi(this);

		pmConnsChtWgt->SetUnitName("n");
		pmDhtChtWgt->SetUnitName("n");

		pmConnsUi->ConnsGbw->layout()->addWidget(dynamic_cast<QWidget *>(pmConnsChtWgt.get()));
		pmConnsUi->DhtGbw->layout()->addWidget(dynamic_cast<QWidget *>(pmDhtChtWgt.get()));

		//------------------
		pmConnsChtWgt->AddDataSet(ChartDrawerData(i18n("Leechers connected"), QPen(StatsPluginSettings::cnLConnColor()), true));
		//

		if (StatsPluginSettings::showLeechersInSwarms())
		{
			pmConnsChtWgt->AddDataSet(ChartDrawerData(i18n("Leechers in swarms"), QPen(StatsPluginSettings::cnLSwarmsColor()), true, *pmLhrSwnUuid));
		}

		//
		pmConnsChtWgt->AddDataSet(ChartDrawerData(i18n("Seeds connected"), QPen(StatsPluginSettings::cnSConnColor()), true));

		//
		if (StatsPluginSettings::showSeedsInSwarms())
		{
			pmConnsChtWgt->AddDataSet(ChartDrawerData(i18n("Seeds in swarms"), QPen(StatsPluginSettings::cnSSwarmsColor()), true, *pmSesSwnUuid));
		}

		//
		pmConnsChtWgt->AddDataSet(ChartDrawerData(i18n("Average leechers connected per torrent"), QPen(StatsPluginSettings::cnAvgLConnPerTorrColor()), true));

		//
		pmConnsChtWgt->AddDataSet(ChartDrawerData(i18n("Average seeds connected per torrent"), QPen(StatsPluginSettings::cnAvgSConnPerTorrColor()), true));

		//
		pmConnsChtWgt->AddDataSet(ChartDrawerData(i18n("Average leechers connected per running torrent"), QPen(StatsPluginSettings::cnAvgLConnPerRunTorrColor()), true));

		//
		pmConnsChtWgt->AddDataSet(ChartDrawerData(i18n("Average seeds connected per running torrent"), QPen(StatsPluginSettings::cnAvgSConnPerRunTorrColor()), true));

		//--------------------------

		if (bt::Globals::instance().getDHT().isRunning())
		{
			pmDhtChtWgt->AddDataSet(ChartDrawerData(i18n("Nodes"), QPen(StatsPluginSettings::dhtNodesColor()), true));
			pmDhtChtWgt->AddDataSet(ChartDrawerData(i18n("Tasks"), QPen(StatsPluginSettings::dhtTasksColor()), true));
		}
		else
		{
			pmConnsUi->DhtGbw->setEnabled(false);
		}

		ApplySettings();
	}

	void ConnsTabPage::ApplySettings()
	{

		pmConnsChtWgt->EnableAntiAlias(StatsPluginSettings::antiAliasing());
		pmDhtChtWgt->EnableAntiAlias(StatsPluginSettings::antiAliasing());

		pmConnsChtWgt->EnableBgdGrid(StatsPluginSettings::drawBgdGrid());
		pmDhtChtWgt->EnableBgdGrid(StatsPluginSettings::drawBgdGrid());


		//-------

		if (StatsPluginSettings::showLeechersInSwarms() && (pmConnsChtWgt->FindUuidInSet(*pmLhrSwnUuid) == -1))
		{
			pmConnsChtWgt->InsertDataSet(1, ChartDrawerData(i18n("Leechers in swarms"), QPen(StatsPluginSettings::cnLSwarmsColor()), true, *pmLhrSwnUuid));
		}

		if ((!StatsPluginSettings::showLeechersInSwarms()) && (pmConnsChtWgt->FindUuidInSet(*pmLhrSwnUuid) != -1))
		{
			pmConnsChtWgt->RemoveDataSet(1);
		}

		//~

		if (StatsPluginSettings::showSeedsInSwarms() && (pmConnsChtWgt->FindUuidInSet(*pmSesSwnUuid) == -1))
		{
			if ((pmConnsChtWgt->FindUuidInSet(*pmLhrSwnUuid) == -1))
			{
				pmConnsChtWgt->InsertDataSet(2, ChartDrawerData(i18n("Seeds in swarms"), QPen(StatsPluginSettings::cnSSwarmsColor()), true, *pmSesSwnUuid));
			}
			else
			{
				pmConnsChtWgt->InsertDataSet(3, ChartDrawerData(i18n("Seeds in swarms"), QPen(StatsPluginSettings::cnSSwarmsColor()), true, *pmSesSwnUuid));
			}
		}

		if ((!StatsPluginSettings::showSeedsInSwarms()) && (pmConnsChtWgt->FindUuidInSet(*pmSesSwnUuid) != -1))
		{
			if ((pmConnsChtWgt->FindUuidInSet(*pmLhrSwnUuid) == -1))
			{
				pmConnsChtWgt->RemoveDataSet(2);
			}
			else
			{
				pmConnsChtWgt->RemoveDataSet(3);
			}
		}

		//-------------

		uint8_t s1, s2;

		s1 = s2 = 1;

		pmConnsChtWgt->SetPen(0, QPen(StatsPluginSettings::cnLConnColor()));

		if (StatsPluginSettings::showLeechersInSwarms())
		{
			pmConnsChtWgt->SetPen(1, QPen(StatsPluginSettings::cnLSwarmsColor()));
			s1 = 0;
		}

		pmConnsChtWgt->SetPen(2 - s1, QPen(StatsPluginSettings::cnSConnColor()));

		if (StatsPluginSettings::showSeedsInSwarms())
		{
			pmConnsChtWgt->SetPen(3 - s1, QPen(StatsPluginSettings::cnSSwarmsColor()));
			s2 = 0;
		}

		pmConnsChtWgt->SetPen(4 - (s1 + s2), QPen(StatsPluginSettings::cnAvgLConnPerTorrColor()));

		pmConnsChtWgt->SetPen(5 - (s1 + s2), QPen(StatsPluginSettings::cnAvgSConnPerTorrColor()));
		pmConnsChtWgt->SetPen(6 - (s1 + s2), QPen(StatsPluginSettings::cnAvgLConnPerRunTorrColor()));
		pmConnsChtWgt->SetPen(7 - (s1 + s2), QPen(StatsPluginSettings::cnAvgSConnPerRunTorrColor()));

		pmDhtChtWgt->SetPen(0, QPen(StatsPluginSettings::dhtNodesColor()));
		pmDhtChtWgt->SetPen(1, QPen(StatsPluginSettings::dhtTasksColor()));

//--------------------------------------------------------------------------------------------------------------------

		pmConnsChtWgt->SetXMax(StatsPluginSettings::connsSamples());

		if (bt::Globals::instance().getDHT().isRunning())
		{
			if (! dynamic_cast<QWidget *>(pmDhtChtWgt.get())->isEnabled())
			{
				pmConnsUi->DhtGbw->setEnabled(true);
			}

			pmDhtChtWgt->SetXMax(StatsPluginSettings::dhtSpdSamples());

		}
		else
		{
			pmConnsUi->DhtGbw->setEnabled(false);
		}

		pmConnsChtWgt->SetXMax(StatsPluginSettings::connsSamples());

		pmDhtChtWgt->SetXMax(StatsPluginSettings::dhtSpdSamples());

		pmConnsChtWgt->SetMaxMode(static_cast<PlainChartDrawer::MaxMode>(StatsPluginSettings::maxMode()));
		pmDhtChtWgt->SetMaxMode(static_cast<PlainChartDrawer::MaxMode>(StatsPluginSettings::maxMode()));
	}

	void ConnsTabPage::UpdateAllCharts()
	{
		pmConnsChtWgt->update();

		if (dynamic_cast<QWidget *>(pmDhtChtWgt.get())->isEnabled())
		{
			pmDhtChtWgt->update();
		}
	}

	void ConnsTabPage::GatherData(Plugin * pPlug)
	{

		GatherConnStats(pPlug);

		if (pmConnsUi->DhtGbw->isEnabled())
		{
			GatherDhtStats();
		}
	}

	void ConnsTabPage::ResetAvg(ChartDrawer *)
	{
	}

	void ConnsTabPage::GatherDhtStats()
	{
		const dht::Stats st = bt::Globals::instance().getDHT().getStats();

		pmDhtChtWgt->AddValue(0, st.num_peers);
		pmDhtChtWgt->AddValue(1, st.num_tasks);
	}

	void ConnsTabPage::GatherConnStats(Plugin * pPlug)
	{
		QueueManager * qm_iface = pPlug->getCore()->getQueueManager();

		if (qm_iface == 0)
		{
			return;
		}

		uint_least32_t lc, ls, sc, ss, tc, rtc;

		lc = ls = sc = ss = tc = rtc = 0;

		for (QList< bt::TorrentInterface *>::iterator it = qm_iface->begin(); it != qm_iface->end(); it++)
		{
			const bt::TorrentStats & tstat = (*it)->getStats();

			lc += tstat.leechers_connected_to;
			ls += tstat.leechers_total;
			sc += tstat.seeders_connected_to;
			ss += tstat.seeders_total;

			tc++;

			if (tstat.running)
			{
				rtc++;
			}
		}

		uint8_t s1, s2;

		s1 = s2 = 1;

		pmConnsChtWgt->AddValue(0, lc);

		if (StatsPluginSettings::showLeechersInSwarms())
		{
			pmConnsChtWgt->AddValue(1, ls);
			s1 = 0;
		}

		pmConnsChtWgt->AddValue(2 - s1, sc);

		if (StatsPluginSettings::showSeedsInSwarms())
		{
			pmConnsChtWgt->AddValue(3 - s1, ss);
			s2 = 0;
		}

		if (tc == 0)
		{
			pmConnsChtWgt->AddValue(4 - (s1 + s2), 0);
			pmConnsChtWgt->AddValue(5 - (s1 + s2), 0);
		}
		else
		{
			pmConnsChtWgt->AddValue(4 - (s1 + s2), static_cast<double>(lc) / static_cast<double>(tc));
			pmConnsChtWgt->AddValue(5 - (s1 + s2), static_cast<double>(sc) / static_cast<double>(tc));
		}

		if (rtc == 0)
		{
			pmConnsChtWgt->AddValue(6 - (s1 + s2), 0);
			pmConnsChtWgt->AddValue(7 - (s1 + s2), 0);
		}
		else
		{
			pmConnsChtWgt->AddValue(6 - (s1 + s2), static_cast<double>(lc) / static_cast<double>(rtc));
			pmConnsChtWgt->AddValue(7 - (s1 + s2), static_cast<double>(sc) / static_cast<double>(rtc));
		}
	}

} //ns end
