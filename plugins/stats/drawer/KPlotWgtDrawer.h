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
#ifndef KPlotWgtDrawer_H_
#define KPlotWgtDrawer_H_

#include <vector>
#include <list>
#include <utility>
#include <memory>
#include <algorithm>

#include <stdint.h>

#include <QWidget>
#include <QList>
#include <QUuid>
#include <QPalette>
#include <QEvent>
#include <QToolTip>
#include <QHelpEvent>
#include <QMenu>
#include <QImage>

#include <kplotwidget.h>
#include <kplotobject.h>
#include <kplotpoint.h>
#include <kplotaxis.h>
#include <QMenu>

#include <drawer/ChartDrawer.h>

namespace kt
{

    /** \brief Drawer class based on KPlotWidget
    \author Krzysztof Kundzicz <athantor@gmail.com>
    */

    class KPlotWgtDrawer : public KPlotWidget, public ChartDrawer
    {
        Q_OBJECT

    public:
        ///Type used as buffer
        typedef std::list<std::pair<size_t, wgtunit_t> > buff_t;
        typedef QList< KPlotObject* > val_t;

    private:
        ///Uuids of plotObjects
        std::vector<QUuid> pmUuids;
        ///Buffer where new values are stored between GUI updates
        buff_t pmBuff;
        ///Descriptions of plotObjects
        std::vector<QString> pmDescs;
        ///Context menu
        QMenu* pmCtxMenu;

        ///Makes a context menu for widget
        void MakeCtxMenu();
        /** \brief Converts ChartDrawerData to KPlotObject
        \param rC ChartDrawerData object to convert
        \return Converted object
        */
        KPlotObject* cdd2kpo(const ChartDrawerData& rC) const;
        ///Adds points to chart from buffer
        void AddPointsFromBuffer();
        ///Marks max
        void MarkMax();


    public:
        ///Constructor
        KPlotWgtDrawer(QWidget* p);

        void paintEvent(QPaintEvent* pPevt);
        QUuid getUuid(const size_t idx) const;

        bool event(QEvent*) ;

    public slots:
        void addValue(const size_t idx, const wgtunit_t val, const bool upd = false);
        void addDataSet(ChartDrawerData Cdd);
        void insertDataSet(const size_t idx, ChartDrawerData Cdd);
        void removeDataSet(const size_t idx);
        void zero(const size_t idx);
        void zeroAll();
        void setUnitName(const QString& rN);
        void setPen(const size_t idx, const QPen& rP);
        void setXMax(const wgtunit_t x);
        void setYMax(const wgtunit_t y);
        void findSetMax();
        void setUuid(const size_t idx, const QUuid& rQ);
        int16_t findUuidInSet(const QUuid& rQ) const;
        void setMaxMode(const MaxMode mm);
        void update();
        void setLegend(const QString& rL);
        QString makeLegendString();

        void enableAntiAlias(bool aa);
        void enableBackgroundGrid(bool bg);

        void showContextMenu(const QPoint& rP);
        void renderToImage();

    signals:
        void Zeroed(ChartDrawer*);

    };

} // ns end

#endif
