/*
    SPDX-FileCopyrightText: 2007 Krzysztof Kundzicz <athantor@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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

    connect(&pmTmr, &QTimer::timeout, this, &StatsPlugin::gatherData);
    connect(getCore(), &CoreInterface::settingsChanged, this, &StatsPlugin::settingsChanged);

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

    disconnect(&pmTmr, &QTimer::timeout, this, &StatsPlugin::gatherData);
    disconnect(getCore(), &CoreInterface::settingsChanged, this, &StatsPlugin::settingsChanged);
}

void StatsPlugin::guiUpdate()
{
    if (mUpdCtr >= StatsPluginSettings::updateEveryGuiUpdates()) {
        pmUiSpd->updateAllCharts();
        pmUiConns->updateAllCharts();

        mUpdCtr = 1;
    } else {
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
