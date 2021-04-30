/*
    SPDX-FileCopyrightText: 2007 Krzysztof Kundzicz <athantor@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef ConnsTabPage_H_
#define ConnsTabPage_H_

#include <QUuid>
#include <QWidget>

#include <dht/dhtbase.h>
#include <interfaces/coreinterface.h>
#include <interfaces/plugin.h>
#include <interfaces/torrentinterface.h>
#include <torrent/globals.h>
#include <torrent/queuemanager.h>

#include <cstdint>
#include <memory>

#include <ui_Conns.h>

#include <PluginPage.h>
#include <drawer/ChartDrawer.h>
#include <drawer/KPlotWgtDrawer.h>
#include <drawer/PlainChartDrawer.h>
#include <statspluginsettings.h>

namespace kt
{
/** \brief Connections tab
\author Krzysztof Kundzicz <athantor@gmail.com>
*/

class ConnsTabPage : public PluginPage
{
    Q_OBJECT

private:
    /// Tab's UI
    Ui::ConnsWgt *pmConnsUi;

    /// Connections chart widget
    ChartDrawer *pmConnsChtWgt;
    /// DHT chart widget
    ChartDrawer *pmDhtChtWgt;

    /** \brief Leechers in swarms dataset UUID

    Used for identification whether this dataset is already shown on the chart and if it needs to be removed or added on settings chage
    */
    const QUuid pmLhrSwnUuid;
    /** \brief Seeds in swarms dataset UUID

    Used for identification whether this dataset is already shown on the chart and if it needs to be removed or added on settings chage
    */
    const QUuid pmSesSwnUuid;

    /** \brief Gathers data about connections
    \param  pPlug kt::Plugin interface
    */
    void GatherConnStats(Plugin *pPlug);
    /** \brief Gathers data about DHT
     */
    void GatherDhtStats();

    void setupUi() override;

public:
    /** \brief Constructor
    \param  p Parent
    */
    ConnsTabPage(QWidget *p);
    /// Destructor
    ~ConnsTabPage() override;

public Q_SLOTS:
    void applySettings() override;
    void updateAllCharts() override;
    void gatherData(Plugin *) override;
    void resetAvg(ChartDrawer *) override;
};

} // ns end

#endif
