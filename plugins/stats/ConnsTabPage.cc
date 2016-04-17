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

    ConnsTabPage::ConnsTabPage(QWidget* p) : PluginPage(p), pmConnsUi(new Ui::ConnsWgt), pmLhrSwnUuid(QUuid::createUuid()),
        pmSesSwnUuid(QUuid::createUuid())
    {

        if (StatsPluginSettings::widgetType() == 0)
        {
            pmConnsChtWgt = new PlainChartDrawer(this);
            pmDhtChtWgt = new PlainChartDrawer(this);
        }
        else if (StatsPluginSettings::widgetType() == 1)
        {
            pmConnsChtWgt = new KPlotWgtDrawer(this);
            pmDhtChtWgt = new KPlotWgtDrawer(this);
        }

        setupUi();
    }

    ConnsTabPage::~ConnsTabPage()
    {
    }


    void ConnsTabPage::setupUi()
    {
        pmConnsUi->setupUi(this);

        pmConnsChtWgt->setUnitName(i18n("Connections"));
        pmDhtChtWgt->setUnitName(i18n("Nodes"));

        pmConnsUi->ConnsGbw->layout()->addWidget(dynamic_cast<QWidget*>(pmConnsChtWgt));
        pmConnsUi->DhtGbw->layout()->addWidget(dynamic_cast<QWidget*>(pmDhtChtWgt));

        //------------------
        pmConnsChtWgt->addDataSet(ChartDrawerData(i18nc("Name of a line on chart", "Leechers connected"), QPen(StatsPluginSettings::cnLConnColor()), true));
        //

        if (StatsPluginSettings::showLeechersInSwarms())
        {
            pmConnsChtWgt->addDataSet(ChartDrawerData(i18nc("Name of a line on chart", "Leechers in swarms"), QPen(StatsPluginSettings::cnLSwarmsColor()), true, pmLhrSwnUuid));
        }

        //
        pmConnsChtWgt->addDataSet(ChartDrawerData(i18nc("Name of a line on chart", "Seeds connected"), QPen(StatsPluginSettings::cnSConnColor()), true));

        //
        if (StatsPluginSettings::showSeedsInSwarms())
        {
            pmConnsChtWgt->addDataSet(ChartDrawerData(i18nc("Name of a line on chart", "Seeds in swarms"), QPen(StatsPluginSettings::cnSSwarmsColor()), true, pmSesSwnUuid));
        }

        //
        pmConnsChtWgt->addDataSet(ChartDrawerData(i18nc("Name of a line on chart", "Average leechers connected per torrent"), QPen(StatsPluginSettings::cnAvgLConnPerTorrColor()), true));

        //
        pmConnsChtWgt->addDataSet(ChartDrawerData(i18nc("Name of a line on chart", "Average seeds connected per torrent"), QPen(StatsPluginSettings::cnAvgSConnPerTorrColor()), true));

        //
        pmConnsChtWgt->addDataSet(ChartDrawerData(i18nc("Name of a line on chart", "Average leechers connected per running torrent"), QPen(StatsPluginSettings::cnAvgLConnPerRunTorrColor()), true));

        //
        pmConnsChtWgt->addDataSet(ChartDrawerData(i18nc("Name of a line on chart", "Average seeds connected per running torrent"), QPen(StatsPluginSettings::cnAvgSConnPerRunTorrColor()), true));

        //--------------------------

        if (bt::Globals::instance().getDHT().isRunning())
        {
            pmDhtChtWgt->addDataSet(ChartDrawerData(i18nc("Name of a line on chart", "Nodes"), QPen(StatsPluginSettings::dhtNodesColor()), true));
            pmDhtChtWgt->addDataSet(ChartDrawerData(i18nc("Name of a line on chart", "Tasks"), QPen(StatsPluginSettings::dhtTasksColor()), true));
        }
        else
        {
            pmConnsUi->DhtGbw->setEnabled(false);
        }

        applySettings();
    }

    void ConnsTabPage::applySettings()
    {

        pmConnsChtWgt->enableAntiAlias(StatsPluginSettings::antiAliasing());
        pmDhtChtWgt->enableAntiAlias(StatsPluginSettings::antiAliasing());

        pmConnsChtWgt->enableBackgroundGrid(StatsPluginSettings::drawBgdGrid());
        pmDhtChtWgt->enableBackgroundGrid(StatsPluginSettings::drawBgdGrid());


        //-------

        if (StatsPluginSettings::showLeechersInSwarms() && (pmConnsChtWgt->findUuidInSet(pmLhrSwnUuid) == -1))
        {
            pmConnsChtWgt->insertDataSet(1, ChartDrawerData(i18nc("Name of a line on chart", "Leechers in swarms"), QPen(StatsPluginSettings::cnLSwarmsColor()), true, pmLhrSwnUuid));
        }

        if ((!StatsPluginSettings::showLeechersInSwarms()) && (pmConnsChtWgt->findUuidInSet(pmLhrSwnUuid) != -1))
        {
            pmConnsChtWgt->removeDataSet(1);
        }

        //~

        if (StatsPluginSettings::showSeedsInSwarms() && (pmConnsChtWgt->findUuidInSet(pmSesSwnUuid) == -1))
        {
            if ((pmConnsChtWgt->findUuidInSet(pmLhrSwnUuid) == -1))
            {
                pmConnsChtWgt->insertDataSet(2, ChartDrawerData(i18nc("Name of a line on chart", "Seeds in swarms"), QPen(StatsPluginSettings::cnSSwarmsColor()), true, pmSesSwnUuid));
            }
            else
            {
                pmConnsChtWgt->insertDataSet(3, ChartDrawerData(i18nc("Name of a line on chart", "Seeds in swarms"), QPen(StatsPluginSettings::cnSSwarmsColor()), true, pmSesSwnUuid));
            }
        }

        if ((!StatsPluginSettings::showSeedsInSwarms()) && (pmConnsChtWgt->findUuidInSet(pmSesSwnUuid) != -1))
        {
            if ((pmConnsChtWgt->findUuidInSet(pmLhrSwnUuid) == -1))
            {
                pmConnsChtWgt->removeDataSet(2);
            }
            else
            {
                pmConnsChtWgt->removeDataSet(3);
            }
        }

        //-------------

        uint8_t s1, s2;

        s1 = s2 = 1;

        pmConnsChtWgt->setPen(0, QPen(StatsPluginSettings::cnLConnColor()));

        if (StatsPluginSettings::showLeechersInSwarms())
        {
            pmConnsChtWgt->setPen(1, QPen(StatsPluginSettings::cnLSwarmsColor()));
            s1 = 0;
        }

        pmConnsChtWgt->setPen(2 - s1, QPen(StatsPluginSettings::cnSConnColor()));

        if (StatsPluginSettings::showSeedsInSwarms())
        {
            pmConnsChtWgt->setPen(3 - s1, QPen(StatsPluginSettings::cnSSwarmsColor()));
            s2 = 0;
        }

        pmConnsChtWgt->setPen(4 - (s1 + s2), QPen(StatsPluginSettings::cnAvgLConnPerTorrColor()));

        pmConnsChtWgt->setPen(5 - (s1 + s2), QPen(StatsPluginSettings::cnAvgSConnPerTorrColor()));
        pmConnsChtWgt->setPen(6 - (s1 + s2), QPen(StatsPluginSettings::cnAvgLConnPerRunTorrColor()));
        pmConnsChtWgt->setPen(7 - (s1 + s2), QPen(StatsPluginSettings::cnAvgSConnPerRunTorrColor()));

        pmDhtChtWgt->setPen(0, QPen(StatsPluginSettings::dhtNodesColor()));
        pmDhtChtWgt->setPen(1, QPen(StatsPluginSettings::dhtTasksColor()));

//--------------------------------------------------------------------------------------------------------------------

        pmConnsChtWgt->setXMax(StatsPluginSettings::connsSamples());

        if (bt::Globals::instance().getDHT().isRunning())
        {
            if (! dynamic_cast<QWidget*>(pmDhtChtWgt)->isEnabled())
            {
                pmConnsUi->DhtGbw->setEnabled(true);
            }

            pmDhtChtWgt->setXMax(StatsPluginSettings::dhtSpdSamples());

        }
        else
        {
            pmConnsUi->DhtGbw->setEnabled(false);
        }

        pmConnsChtWgt->setXMax(StatsPluginSettings::connsSamples());

        pmDhtChtWgt->setXMax(StatsPluginSettings::dhtSpdSamples());

        pmConnsChtWgt->setMaxMode(static_cast<PlainChartDrawer::MaxMode>(StatsPluginSettings::maxMode()));
        pmDhtChtWgt->setMaxMode(static_cast<PlainChartDrawer::MaxMode>(StatsPluginSettings::maxMode()));
    }

    void ConnsTabPage::updateAllCharts()
    {
        pmConnsChtWgt->update();

        if (dynamic_cast<QWidget*>(pmDhtChtWgt)->isEnabled())
        {
            pmDhtChtWgt->update();
        }
    }

    void ConnsTabPage::gatherData(Plugin* pPlug)
    {

        GatherConnStats(pPlug);

        if (pmConnsUi->DhtGbw->isEnabled())
        {
            GatherDhtStats();
        }
    }

    void ConnsTabPage::resetAvg(ChartDrawer*)
    {
    }

    void ConnsTabPage::GatherDhtStats()
    {
        const dht::Stats st = bt::Globals::instance().getDHT().getStats();

        pmDhtChtWgt->addValue(0, st.num_peers);
        pmDhtChtWgt->addValue(1, st.num_tasks);
    }

    void ConnsTabPage::GatherConnStats(Plugin* pPlug)
    {
        QueueManager* qm_iface = pPlug->getCore()->getQueueManager();

        if (qm_iface == 0)
        {
            return;
        }

        uint_least32_t lc, ls, sc, ss, tc, rtc;

        lc = ls = sc = ss = tc = rtc = 0;

        for (QList< bt::TorrentInterface*>::iterator it = qm_iface->begin(); it != qm_iface->end(); it++)
        {
            const bt::TorrentStats& tstat = (*it)->getStats();

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

        pmConnsChtWgt->addValue(0, lc);

        if (StatsPluginSettings::showLeechersInSwarms())
        {
            pmConnsChtWgt->addValue(1, ls);
            s1 = 0;
        }

        pmConnsChtWgt->addValue(2 - s1, sc);

        if (StatsPluginSettings::showSeedsInSwarms())
        {
            pmConnsChtWgt->addValue(3 - s1, ss);
            s2 = 0;
        }

        if (tc == 0)
        {
            pmConnsChtWgt->addValue(4 - (s1 + s2), 0);
            pmConnsChtWgt->addValue(5 - (s1 + s2), 0);
        }
        else
        {
            pmConnsChtWgt->addValue(4 - (s1 + s2), static_cast<double>(lc) / static_cast<double>(tc));
            pmConnsChtWgt->addValue(5 - (s1 + s2), static_cast<double>(sc) / static_cast<double>(tc));
        }

        if (rtc == 0)
        {
            pmConnsChtWgt->addValue(6 - (s1 + s2), 0);
            pmConnsChtWgt->addValue(7 - (s1 + s2), 0);
        }
        else
        {
            pmConnsChtWgt->addValue(6 - (s1 + s2), static_cast<double>(lc) / static_cast<double>(rtc));
            pmConnsChtWgt->addValue(7 - (s1 + s2), static_cast<double>(sc) / static_cast<double>(rtc));
        }
    }

} //ns end
