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
#ifndef PluginPage_H_
#define PluginPage_H_

#include <QWidget>
#include <utility>

#include <interfaces/plugin.h>
#include <drawer/ChartDrawer.h>

namespace kt
{

    /** \brief Base class for plugin's tabs in the main UI
    \author Krzysztof Kundzicz <athantor@gmail.com>
    */

    class PluginPage : public QWidget
    {
        Q_OBJECT
    public:
        //sum , msmnts
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
        PluginPage(QWidget* p);
        ///Destructor
        ~PluginPage();

    public slots:
        ///Applies settings
        virtual void applySettings() = 0;
        ///Updates all charts
        virtual void updateAllCharts() = 0;
        /** \brief Gathers data
        \param pP Plugin interface
        */
        virtual void gatherData(Plugin* pP) = 0;
        ///Resets average
        virtual void resetAvg(ChartDrawer*) = 0;

    protected:
        ///Setups UI
        virtual void setupUi() = 0;
    };

} // ns end

#endif
