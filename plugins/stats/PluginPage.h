/*
    SPDX-FileCopyrightText: 2007 Krzysztof Kundzicz <athantor@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef PluginPage_H_
#define PluginPage_H_

#include <QWidget>
#include <utility>

#include <drawer/ChartDrawer.h>
#include <interfaces/plugin.h>

namespace kt
{
/** \brief Base class for plugin's tabs in the main UI
\author Krzysztof Kundzicz <athantor@gmail.com>
*/

class PluginPage : public QWidget
{
    Q_OBJECT
public:
    // sum , msmnts
    /** \brief Type used for computing average
     *
     * Layout:
     * - First: Sum of measurements
     * - Second: Amount
     */
    typedef std::pair<long double, long double> avg_t;

    /** \brief Constructor
    \param p Parent
    */
    PluginPage(QWidget *p);
    /// Destructor
    ~PluginPage() override;

public Q_SLOTS:
    /// Applies settings
    virtual void applySettings() = 0;
    /// Updates all charts
    virtual void updateAllCharts() = 0;
    /** \brief Gathers data
    \param pP Plugin interface
    */
    virtual void gatherData(Plugin *pP) = 0;
    /// Resets average
    virtual void resetAvg(ChartDrawer *) = 0;

protected:
    /// Setups UI
    virtual void setupUi() = 0;
};

} // ns end

#endif
