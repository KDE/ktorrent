/*
    SPDX-FileCopyrightText: 2007 Krzysztof Kundzicz <athantor@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SpdTabPage_H_
#define SpdTabPage_H_

#include <QList>
#include <QPen>
#include <QString>
#include <QWidget>

#include <interfaces/coreinterface.h>
#include <interfaces/peerinterface.h>
#include <interfaces/plugin.h>
#include <interfaces/torrentinterface.h>
#include <peer/peer.h>
#include <peer/peermanager.h>
#include <settings.h>
#include <torrent/queuemanager.h>
#include <torrent/torrentcontrol.h>

#include <cstdint>
#include <memory>

#include <PluginPage.h>
#include <drawer/ChartDrawer.h>
#include <drawer/ChartDrawerData.h>
#include <drawer/KPlotWgtDrawer.h>
#include <drawer/PlainChartDrawer.h>
#include <statspluginsettings.h>

#include <ui_Spd.h>

namespace kt
{
/** \brief Speeds tab
\author Krzysztof Kundzicz <athantor@gmail.com>
*/

class SpdTabPage : public PluginPage
{
    Q_OBJECT
public:
    /** \brief Constructor
    \param p Parent
    */
    SpdTabPage(QWidget *p);
    /// Destructor
    ~SpdTabPage() override;

public Q_SLOTS:
    void applySettings() override;
    void updateAllCharts() override;
    void gatherData(Plugin *) override;
    void resetAvg(ChartDrawer *) override;

private:
    /** \brief Gathers dl speeds data
     \ param  pP kt::Plugin interfac*e *
     */
    void gatherDownloadSpeed(Plugin *pP);
    /** \brief Gathers peers speeds data
     \ param  pP kt::Plugin interfac*e *
     */
    void gatherPeersSpeed(Plugin *pP);
    /** \brief Gathers Ul speeds data
     \ param  pP kt::Plugin interfac*e *
     */
    void gatherUploadSpeed(Plugin *pP);

    void setupUi() override;

private:
    /// Page's UI
    Ui::SpdWgt *pmUiSpd;

    /// Dl speeds chart widget
    ChartDrawer *pmDlChtWgt;
    /// Peers speeds chart widget
    ChartDrawer *pmPeersChtWgt;
    /// Ul speeds chart widget
    ChartDrawer *pmUlChtWgt;

    /// Dl average
    avg_t mDlAvg;
    /// Ul average
    avg_t mUlAvg;
};

} // ns end

#endif
