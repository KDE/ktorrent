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

K_EXPORT_COMPONENT_FACTORY(ktstatsplugin, KGenericFactory<kt::StatsPlugin>("ktstatsplugin"))

namespace kt
{

    StatsPlugin::StatsPlugin(QObject* p, const QStringList&) : Plugin(p), mUpdCtr(1)
    {
        pmUiSett = 0;
        pmDispSett = 0;
    }

    StatsPlugin::~StatsPlugin()
    {
    }

    void StatsPlugin::load()
    {
        pmUiSpd.release();
        pmUiConns.release();
        pmTmr.release();

        pmUiSpd.reset(new SpdTabPage(0));
        pmUiConns.reset(new ConnsTabPage(0));
        pmUiSett = new SettingsPage(0);
        pmDispSett = new DisplaySettingsPage(0);
        pmTmr.reset(new QTimer(this));

        TorrentActivityInterface* ta = getGUI()->getTorrentActivity();
        ta->addToolWidget(pmUiSpd.get(), i18n("Speed charts"), "view-statistics", i18n("Displays charts about download and upload speed"));
        ta->addToolWidget(pmUiConns.get(), i18n("Connections charts"), "view-statistics", i18n("Displays charts about connections"));

        getGUI()->addPrefPage(pmUiSett);
        getGUI()->addPrefPage(pmDispSett);

        connect(pmTmr.get(), SIGNAL(timeout()), dynamic_cast<StatsPlugin*>(this), SLOT(gatherData()));
        connect(getCore(), SIGNAL(settingsChanged()), this, SLOT(settingsChanged()));

        pmTmr->start(StatsPluginSettings::dataGatherIval());

    }

    void StatsPlugin::unload()
    {
        TorrentActivityInterface* ta = getGUI()->getTorrentActivity();
        ta->removeToolWidget(pmUiSpd.get());
        ta->removeToolWidget(pmUiConns.get());

        getGUI()->removePrefPage(pmUiSett);
        getGUI()->removePrefPage(pmDispSett);

        pmTmr->stop();

        disconnect(pmTmr.get());
        disconnect(getCore());

        pmUiSpd.reset();
        pmUiConns.reset();

        pmUiSett = 0;
        pmDispSett = 0;

        pmTmr.reset();

    }

    bool StatsPlugin::versionCheck(const QString& version) const
    {
        return version == KT_VERSION_MACRO;
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
        pmTmr->setInterval(StatsPluginSettings::dataGatherIval());
        pmUiSpd->applySettings();
        pmUiConns->applySettings();
    }

} //Ns end
