/*
    SPDX-FileCopyrightText: 2007 Krzysztof Kundzicz <athantor@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <KPluginFactory>
#include <StatsPlugin.h>
#include <interfaces/torrentactivityinterface.h>

K_PLUGIN_CLASS_WITH_JSON(kt::StatsPlugin, "ktorrent_stats.json")

namespace kt
{
StatsPlugin::StatsPlugin(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : Plugin(parent, data, args)
    , mUpdCtr(1)
{
    pmUiSett = nullptr;
    pmDispSett = nullptr;
}

StatsPlugin::~StatsPlugin()
{
}

void StatsPlugin::load()
{
    pmUiSpd = std::make_unique<SpdTabPage>(nullptr);
    pmUiConns = std::make_unique<ConnsTabPage>(nullptr);
    pmUiSett = std::make_unique<SettingsPage>(nullptr);
    pmDispSett = std::make_unique<DisplaySettingsPage>(nullptr);

    TorrentActivityInterface *ta = getGUI()->getTorrentActivity();
    ta->addToolWidget(pmUiSpd.get(), i18n("Speed charts"), QStringLiteral("view-statistics"), i18n("Displays charts about download and upload speed"));
    ta->addToolWidget(pmUiConns.get(), i18n("Connections charts"), QStringLiteral("view-statistics"), i18n("Displays charts about connections"));

    getGUI()->addPrefPage(pmUiSett.get());
    getGUI()->addPrefPage(pmDispSett.get());

    connect(&pmTmr, &QTimer::timeout, this, &StatsPlugin::gatherData);
    connect(getCore(), &CoreInterface::settingsChanged, this, &StatsPlugin::settingsChanged);

    pmTmr.start(StatsPluginSettings::dataGatherIval());
}

void StatsPlugin::unload()
{
    TorrentActivityInterface *ta = getGUI()->getTorrentActivity();
    ta->removeToolWidget(pmUiSpd.get());
    ta->removeToolWidget(pmUiConns.get());

    getGUI()->removePrefPage(pmUiSett.get());
    getGUI()->removePrefPage(pmDispSett.get());

    pmTmr.stop();

    disconnect(&pmTmr, &QTimer::timeout, this, &StatsPlugin::gatherData);
    disconnect(getCore(), &CoreInterface::settingsChanged, this, &StatsPlugin::settingsChanged);

    pmDispSett.reset();
    pmUiSett.reset();
    pmUiConns.reset();
    pmUiSpd.reset();
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

} // Ns end

#include "StatsPlugin.moc"

#include "moc_StatsPlugin.cpp"
