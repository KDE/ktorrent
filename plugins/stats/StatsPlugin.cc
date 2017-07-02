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

#include <StatsPlugin.h>
#include <interfaces/torrentactivityinterface.h>
#include <KPluginFactory>

K_PLUGIN_FACTORY_WITH_JSON(ktorrent_stats, "ktorrent_stats.json", registerPlugin<kt::StatsPlugin>();)

namespace kt
{

    StatsPlugin::StatsPlugin(QObject* p, const QVariantList&) : Plugin(p), mUpdCtr(1)
    {
        pmUiSett = nullptr;
        pmDispSett = nullptr;
    }

    StatsPlugin::~StatsPlugin()
    {
    }

    void StatsPlugin::load()
    {
        pmUiSpd = new SpdTabPage(nullptr);
        pmUiConns = new ConnsTabPage(nullptr);
        pmUiSett = new SettingsPage(nullptr);
        pmDispSett = new DisplaySettingsPage(nullptr);

        TorrentActivityInterface* ta = getGUI()->getTorrentActivity();
        ta->addToolWidget(pmUiSpd, i18n("Speed charts"), QStringLiteral("view-statistics"), i18n("Displays charts about download and upload speed"));
        ta->addToolWidget(pmUiConns, i18n("Connections charts"), QStringLiteral("view-statistics"), i18n("Displays charts about connections"));

        getGUI()->addPrefPage(pmUiSett);
        getGUI()->addPrefPage(pmDispSett);

        connect(&pmTmr, SIGNAL(timeout()), dynamic_cast<StatsPlugin*>(this), SLOT(gatherData()));
        connect(getCore(), SIGNAL(settingsChanged()), this, SLOT(settingsChanged()));

        pmTmr.start(StatsPluginSettings::dataGatherIval());

    }

    void StatsPlugin::unload()
    {
        TorrentActivityInterface* ta = getGUI()->getTorrentActivity();
        ta->removeToolWidget(pmUiSpd);
        ta->removeToolWidget(pmUiConns);

        getGUI()->removePrefPage(pmUiSett);
        getGUI()->removePrefPage(pmDispSett);

        pmTmr.stop();

        disconnect(&pmTmr);
        disconnect(getCore());
    }

    bool StatsPlugin::versionCheck(const QString& version) const
    {
        return version == QStringLiteral(KT_VERSION_MACRO);
    }

    void StatsPlugin::guiUpdate()
    {
        if (mUpdCtr >= StatsPluginSettings::updateEveryGuiUpdates())
        {
            pmUiSpd->updateAllCharts();
            pmUiConns->updateAllCharts();

            mUpdCtr = 1;
        }
        else
        {
            mUpdCtr++;
        }
    }

    void StatsPlugin::gatherData()
    {
        pmUiSpd->gatherData(this);
        pmUiConns->gatherData(this);
    }

    void StatsPlugin::settingsChanged()
    {
        pmTmr.setInterval(StatsPluginSettings::dataGatherIval());
        pmUiSpd->applySettings();
        pmUiConns->applySettings();
    }

} //Ns end

#include "StatsPlugin.moc"
