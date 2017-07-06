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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#ifndef ChartDrawer_H_
#define ChartDrawer_H_

#include <QString>
#include <QPen>
#include <QUuid>
#include <QPaintEvent>

#include <memory>
#include <cstdint>

#include <ChartDrawerData.h>

namespace kt
{

    /**
    \brief Base class for chart widgets used by plugin
    \author Krzysztof Kundzicz <athantor@gmail.com>
    */

    class ChartDrawer
    {

    public:
        ///Mode of setting maximum on the chart's OY axis
        enum MaxMode
        {
            MM_Top, ///< Max ever achieved
            MM_Exact ///< Max visible on chart
        };

        ///Type used as values on charts
        typedef qreal wgtunit_t;
        ///Type holding chart's data values
        typedef std::vector< ChartDrawerData > val_t;

        /** \brief Constructor
        */
        ChartDrawer();
        ///Destructor
        virtual ~ChartDrawer();

        /** \brief Get maximum on OX
        \return Max on OX
        */
        wgtunit_t getXMax() const;
        /** \brief Get maximum on OY
        \return Max on OY
        */
        wgtunit_t getYMax() const;
        /** \brief Get name of a unit used shown chart
        \return Name
        */
        QString getUnitName() const {return pmUnitName;}
        /** \brief Get UUID of the dataset
        \param idx Index of the dataset
        \return UUID
        */
        virtual QUuid getUuid(const size_t idx) const = 0;

        /** \brief Gets the begin of dataset as const_iterator
        \return Beginning of the set
        */
        virtual val_t::const_iterator begin() const;
        /** \brief Gets the end of dataset as const_iterator
        \return End of the set
        */
        virtual val_t::const_iterator end() const;

        /** \brief QWidget's paintEvent
        \param pPevt Event

        Implement here the drawing of the chart.
        */
        virtual void paintEvent(QPaintEvent* pPevt) = 0;

        /** \brief Amount of dataset shown on chart
        \return Datasets count
        */
        size_t dataSetCount() const;
        //---------------
        /** \brief Adds value to the dataset
        \param idx Index of the dataset
        \param val Value that's being added
        \param upd Update chart after adding?
        */
        virtual void addValue(const size_t idx, const wgtunit_t val, const bool upd = false) = 0;
        /** \brief Add new dataset
        \param Cdd Dataset that's being added
        */
        virtual void addDataSet(ChartDrawerData Cdd) = 0;
        /** \brief Inserts dataset before idx
        \param idx Index of the dataset before which new set is boing to be added
        \param Cdd New dataset
        */
        virtual void insertDataSet(const size_t idx, ChartDrawerData Cdd) = 0;
        /** \brief Removes dataset
        \param idx Index of the dataset to remove
        */
        virtual void removeDataSet(const size_t idx) = 0;
        /** \brief Zeroes dataset
        \param idx Index of the dataset to zero
        */
        virtual void zero(const size_t idx) = 0;
        ///Zeroes all datasets in the chart
        virtual void zeroAll() = 0;
        /** \brief Sets maximum mode
        \param mm Mode
        */
        virtual void setMaxMode(const MaxMode mm) = 0;
        /** \brief Sets name of an unit shown on the chart
        \param rN Unit's name
        */
        virtual void setUnitName(const QString& rN) = 0;
        /** \brief Sets pen of the dataset
        \param idx Index of the dataset which pen is beeng changed
        \param rP New pen
        */
        virtual void setPen(const size_t idx, const QPen& rP) = 0;
        /** \brief Sets UUID of the dataset
        \param idx Index of the dataset which pen is beeng changed
        \param rQ New UUID
        */
        virtual void setUuid(const size_t idx, const QUuid& rQ) = 0;
        /** \brief Finds given UUID in the set
        \param rQ UUID to find
        \return Index of the set
        \retval -1 UUID not found
        \retval >0 Found index
        */
        virtual int16_t findUuidInSet(const QUuid& rQ) const = 0;
        /** \brief Enable antialiasing
        \param aa Enable?
        */
        virtual void enableAntiAlias(bool aa) = 0;

        /** \brief Enable background grid
        \param bg Enable?
        */
        virtual void enableBackgroundGrid(bool bg) = 0;

        /** \brief Sets maximum on OX axis
        \param x Maximum
        */
        virtual void setXMax(const wgtunit_t x) = 0;
        /** \brief Sets maximum on OY axis
        \param y Maximum
        */
        virtual void setYMax(const wgtunit_t y) = 0;

        ///Automagically finds maximum from data in sets and sets it on OY scale
        virtual void findSetMax() = 0;

        ///Function generating legend string
        virtual QString makeLegendString() = 0;

        /** \brief Function settng a legend
        \param rL Legend's text
        */
        virtual void setLegend(const QString& rL) = 0;
        ///Updates the widget
        virtual void update() = 0;

        /** \brief Shows context menu @ point
        \param rP Point where to show menu
        */
        virtual void showContextMenu(const QPoint& rP) = 0;
        /** \brief Renders chart to image
        \note This function will show modal KFileDialog
        */
        virtual void renderToImage() = 0;

    protected:
        ///Pointer to chart's data container
        val_t pmVals;
        ///Pointer to a name of the unit used on chart
        QString pmUnitName;
        ///Current maximum mode
        MaxMode mCurrMaxMode;
        ///Current maximum on OX
        wgtunit_t mXMax;
        ///Current maximum on OY
        wgtunit_t mYMax;
        ///Use antialiasing?
        bool mAntiAlias;
        ///Draw bgd grid?
        bool mBgdGrid;
    };

} // ns end

#endif
