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

namespace kt
{

    SpdTabPage::SpdTabPage(QWidget* p) : PluginPage(p), pmUiSpd(new Ui::SpdWgt), mDlAvg(std::make_pair(0, 0)), mUlAvg(std::make_pair(0, 0))
    {

        if (StatsPluginSettings::widgetType() == 0)
        {
            pmDlChtWgt = new PlainChartDrawer(this);
            pmPeersChtWgt = new PlainChartDrawer(this);
            pmUlChtWgt = new PlainChartDrawer(this);

            connect(dynamic_cast<PlainChartDrawer*>(pmDlChtWgt), SIGNAL(Zeroed(ChartDrawer*)), this, SLOT(resetAvg(ChartDrawer*)));
            connect(dynamic_cast<PlainChartDrawer*>(pmUlChtWgt), SIGNAL(Zeroed(ChartDrawer*)), this, SLOT(resetAvg(ChartDrawer*)));

        }
        else if (StatsPluginSettings::widgetType() == 1)
        {
            pmDlChtWgt = new KPlotWgtDrawer(this);
            pmPeersChtWgt = new KPlotWgtDrawer(this);
            pmUlChtWgt = new KPlotWgtDrawer(this);

            connect(dynamic_cast<KPlotWgtDrawer*>(pmDlChtWgt), SIGNAL(Zeroed(ChartDrawer*)), this, SLOT(resetAvg(ChartDrawer*)));
            connect(dynamic_cast<KPlotWgtDrawer*>(pmUlChtWgt), SIGNAL(Zeroed(ChartDrawer*)), this, SLOT(resetAvg(ChartDrawer*)));
        }



        setupUi();
    }

    SpdTabPage::~SpdTabPage()
    {
    }

    void SpdTabPage::setupUi()
    {
        pmUiSpd->setupUi(this);

        pmUiSpd->DlSpdGbw->layout()->addWidget(dynamic_cast<QWidget*>(pmDlChtWgt));
        pmUiSpd->PeersSpdGbw->layout()->addWidget(dynamic_cast<QWidget*>(pmPeersChtWgt));
        pmUiSpd->UlSpdGbw->layout()->addWidget(dynamic_cast<QWidget*>(pmUlChtWgt));

        pmDlChtWgt->addDataSet(ChartDrawerData(i18nc("Name of a line on download chart", "Current speed"), QPen(StatsPluginSettings::dlSpdColor()), true));
        pmUlChtWgt->addDataSet(ChartDrawerData(i18nc("Name of a line on upload chart", "Current speed"), QPen(StatsPluginSettings::ulSpdColor()), true));

        pmDlChtWgt->addDataSet(ChartDrawerData(i18nc("Name of a line on download chart", "Average speed"), QPen(StatsPluginSettings::dlAvgColor()), true));
        pmUlChtWgt->addDataSet(ChartDrawerData(i18nc("Name of a line on upload chart", "Average speed"), QPen(StatsPluginSettings::ulAvgColor()), true));

        pmDlChtWgt->addDataSet(ChartDrawerData(i18nc("Name of a line on download chart", "Speed limit"), QPen(StatsPluginSettings::dlLimitColor()), true));
        pmUlChtWgt->addDataSet(ChartDrawerData(i18nc("Name of a line on upload chart", "Speed limit"), QPen(StatsPluginSettings::ulLimitColor()), true));

        pmPeersChtWgt->addDataSet(ChartDrawerData(i18nc("Name of a line on chart", "Average from leechers"), QPen(StatsPluginSettings::prAvgFromLColor()), true));
        pmPeersChtWgt->addDataSet(ChartDrawerData(i18nc("Name of a line on chart", "Average to leechers"), QPen(StatsPluginSettings::prAvgToLColor()), true));
        pmPeersChtWgt->addDataSet(ChartDrawerData(i18nc("Name of a line on chart", "Average from seeds"), QPen(StatsPluginSettings::prAvgFromSColor()), true));
        pmPeersChtWgt->addDataSet(ChartDrawerData(i18nc("Name of a line on chart", "From leechers"), QPen(StatsPluginSettings::prFromLColor()), true));
        pmPeersChtWgt->addDataSet(ChartDrawerData(i18nc("Name of a line on chart", "From seeds"), QPen(StatsPluginSettings::prFromSColor()), true));

        applySettings();
    }

    void SpdTabPage::applySettings()
    {

        pmDlChtWgt->setPen(0, QPen(StatsPluginSettings::dlSpdColor()));
        pmUlChtWgt->setPen(0, QPen(StatsPluginSettings::ulSpdColor()));

        pmDlChtWgt->setPen(1, QPen(StatsPluginSettings::dlAvgColor()));
        pmUlChtWgt->setPen(1, QPen(StatsPluginSettings::ulAvgColor()));

        pmDlChtWgt->setPen(2, QPen(StatsPluginSettings::dlLimitColor()));
        pmUlChtWgt->setPen(2, QPen(StatsPluginSettings::ulLimitColor()));

        pmPeersChtWgt->setPen(0, QPen(StatsPluginSettings::prAvgFromLColor()));
        pmPeersChtWgt->setPen(1, QPen(StatsPluginSettings::prAvgToLColor()));
        pmPeersChtWgt->setPen(2, QPen(StatsPluginSettings::prAvgFromSColor()));
        pmPeersChtWgt->setPen(3, QPen(StatsPluginSettings::prFromLColor()));
        pmPeersChtWgt->setPen(4, QPen(StatsPluginSettings::prFromSColor()));

        pmDlChtWgt->setXMax(StatsPluginSettings::dlSpdSamples());
        pmPeersChtWgt->setXMax(StatsPluginSettings::peersSpdSamples());
        pmUlChtWgt->setXMax(StatsPluginSettings::ulSpdSamples());

        pmDlChtWgt->setMaxMode(static_cast<ChartDrawer::MaxMode>(StatsPluginSettings::maxMode()));
        pmPeersChtWgt->setMaxMode(static_cast<ChartDrawer::MaxMode>(StatsPluginSettings::maxMode()));
        pmUlChtWgt->setMaxMode(static_cast<ChartDrawer::MaxMode>(StatsPluginSettings::maxMode()));

        // ---

        pmDlChtWgt->enableAntiAlias(StatsPluginSettings::antiAliasing());
        pmPeersChtWgt->enableAntiAlias(StatsPluginSettings::antiAliasing());
        pmUlChtWgt->enableAntiAlias(StatsPluginSettings::antiAliasing());

        pmDlChtWgt->enableBackgroundGrid(StatsPluginSettings::drawBgdGrid());
        pmPeersChtWgt->enableBackgroundGrid(StatsPluginSettings::drawBgdGrid());
        pmUlChtWgt->enableBackgroundGrid(StatsPluginSettings::drawBgdGrid());

    }

    void SpdTabPage::updateAllCharts()
    {
        pmDlChtWgt->update();
        pmPeersChtWgt->update();
        pmUlChtWgt->update();
    }

    void SpdTabPage::gatherDownloadSpeed(Plugin* pPlug)
    {
        uint spd = pPlug->getCore()->getStats().download_speed;
        mDlAvg.first += spd;
        mDlAvg.second++;

        pmDlChtWgt->addValue(0,  spd / 1024.0);
        pmDlChtWgt->addValue(1, (mDlAvg.first / mDlAvg.second) / 1024.0);
        pmDlChtWgt->addValue(2, Settings::maxDownloadRate());
    }

    void SpdTabPage::gatherPeersSpeed(Plugin* pPlug)
    {
        kt::QueueManager* qm_iface = pPlug->getCore()->getQueueManager();

        if (qm_iface == 0)
        {
            return;
        }

        uint_least64_t l_up_spd, l_dn_spd, s_dn_spd;

        uint_least32_t l_cnt, s_cnt;

        l_up_spd = l_dn_spd = s_dn_spd = l_cnt = s_cnt = 0;

        for (QList< bt::TorrentInterface*>::iterator it = qm_iface->begin(); it != qm_iface->end(); it++)
        {
            bt::TorrentControl* tctl = dynamic_cast<bt::TorrentControl*>(*it);

            if (!tctl)
            {
                continue;
            }

            const bt::PeerManager* p_mgr = tctl->getPeerMgr();

            QList<bt::Peer::Ptr> ppl = p_mgr->getPeers();

            foreach (bt::Peer::Ptr peer, ppl)
            {
                const bt::PeerInterface::Stats p_stats = peer->getStats();

                if (p_stats.perc_of_file >= 100)
                {
                    s_dn_spd += p_stats.download_rate;
                    s_cnt++;

                }
                else
                {
                    l_dn_spd += p_stats.download_rate;
                    l_up_spd += p_stats.upload_rate;
                    l_cnt++;
                }
            }
        }

        if (!l_cnt)
        {
            pmPeersChtWgt->addValue(0, 0);
            pmPeersChtWgt->addValue(1, 0);
            pmPeersChtWgt->addValue(3, 0);
        }
        else
        {
            pmPeersChtWgt->addValue(0, (static_cast<double>(l_dn_spd) / static_cast<double>(l_cnt)) / 1024.0);
            pmPeersChtWgt->addValue(1, (static_cast<double>(l_up_spd) / static_cast<double>(l_cnt)) / 1024.0);
            pmPeersChtWgt->addValue(3, static_cast<double>(l_dn_spd) / 1024.0);
        }

        if (!s_cnt)
        {
            pmPeersChtWgt->addValue(2, 0);
            pmPeersChtWgt->addValue(4, 0);
        }
        else
        {
            pmPeersChtWgt->addValue(2, (static_cast<double>(s_dn_spd) / static_cast<double>(s_cnt)) / 1024.0);
            pmPeersChtWgt->addValue(4, static_cast<double>(s_dn_spd) / 1024.0);
        }


    }

    void SpdTabPage::gatherUploadSpeed(Plugin* pPlug)
    {
        uint spd = pPlug->getCore()->getStats().upload_speed;
        mUlAvg.first += spd;
        mUlAvg.second++;

        pmUlChtWgt->addValue(0, spd / 1024.0);
        pmUlChtWgt->addValue(1, (mUlAvg.first / mUlAvg.second) / 1024.0);
        pmUlChtWgt->addValue(2, Settings::maxUploadRate());
    }

    void SpdTabPage::gatherData(Plugin* pPlug)
    {
        gatherDownloadSpeed(pPlug);
        gatherPeersSpeed(pPlug);
        gatherUploadSpeed(pPlug);
    }

    void SpdTabPage::resetAvg(ChartDrawer* c)
    {
        if (!c)
        {
            return;
        }
        else if (c == pmDlChtWgt)
        {
            mDlAvg = std::make_pair(0, 0);
        }
        else if (c == pmUlChtWgt)
        {
            mUlAvg = std::make_pair(0, 0);
        }
        else
        {
            qDebug("Got unreckognized widget!");
        }
    }

} //ns e
