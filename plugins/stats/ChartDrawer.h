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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef CHARTDRAWER_H_
#define CHARTDRAWER_H_

#include <qwidget.h>
#include <qpainter.h>
#include <qstring.h>
#include <qtooltip.h>
#include <qmime.h>
#include <qimage.h>

#include <klocale.h>

#include <vector>
#include <cmath>
#include <algorithm> //fill

#include "ChartDrawerData.h"

namespace kt {

/**
\brief Widget for drawing line charts
\author Krzysztof Kundzicz <athantor@gmail.com>
*/
class ChartDrawer : public QWidget
{
	Q_OBJECT
	
	public:
		///Type used as widget size unit
		typedef uint32_t wgtsize_t;
		///Type used as unit in chart
		typedef int64_t wgtunit_t;
		/**
		\brief Type used for data storing
		\sa ChartDrawerData
		*/
		typedef std::vector<ChartDrawerData> val_t;
		///Determines max mode
		enum MaxMode { MaxModeTop, MaxModeExact };

	private:
		///Maximum X value
		wgtsize_t mXMax;
		///Maximum Y value
		wgtsize_t mYMax;
		///Auto maximum setting
		bool mAutoMax;
		///Chart data
		val_t mEls;
		///Name of the chart unit
		QString mUnitName;
		///Mark max
		std::vector<bool> mMarkMax;
		///Max mode
		MaxMode mMMode;
	
		///Paint event handler
		void paintEvent ( QPaintEvent * );
		/**
		\brief Draws chart's frame
		\param rPnt Painter on which things will be drawn
		*/
		void DrawFrame(QPainter &rPnt);
		/**
		\brief Draws chart's scale
		\param rPnt Painter on which things will be drawn
		*/
		void DrawScale(QPainter &rPnt);
		/**
		\brief Draws chart
		\param rPnt Painter on which things will be drawn
		*/
		void DrawChart(QPainter &rPnt);
		
		/**
		\brief Gets distance between two values on OY
		\return Distance
		*/
		inline wgtsize_t GetYScale() const;
		
		/**
		\brief Translates widget Y coord to cartesian
		\param y Coord
		\return Coord
		*/
		inline wgtunit_t TrY(const wgtunit_t y) const;
		/**
		\brief Returns charts height
		\return Height
		
		Return only height of the chart's inside the frame — not the whole widget's
		*/
		inline wgtunit_t height() const;
		/**
		\brief Returns charts width
		\return Width
		
		Return only width of the chart's inside the frame — not the whole widget's
		*/
		inline wgtunit_t width() const;
		
		/**
		\brief Finds screen X coord on the widget
		\param x Coord
		\return Screen coord
		\warning Thera are rounding errors
		*/
		inline wgtunit_t FindXScreenCoords(const double x) const;
		/**
		\brief Finds screen Y coord on the widget
		\param y Coord
		\return Screen coord
		\warning Thera are rounding errors
		*/
		inline wgtunit_t FindYScreenCoords(const double y) const;
		
		///Sets tooltip with legend
		void MakeLegendTooltip();
		
	public:
		/**
		\brief Widget's constructor
		\param p Parent
		\param x_max Maximum X size
		\param y_max Maximum Y size
		\param autom Whether athomagically set the maximum Y size
		\param uname Unit name
		*/
		ChartDrawer(QWidget *p = 0, wgtsize_t x_max = 2, wgtsize_t y_max = 1, bool autom = true, const QString & uname = "KB/s");
		~ChartDrawer();
		
		/**
		\brief Gets maximum X
		\return Maximum X
		*/
		wgtsize_t GetXMax() const;
		/**
		\brief Gets maximum Y
		\return Maximum Y
		*/
		wgtsize_t GetYMax() const;
		
		/**
		\brief Sets the units name
		\param rN Name
		
		\note It'l be drawn on the chart
		*/
		void SetUnitName(const QString & rN);
		
		/**
		\brief Gets unit name
		\return name
		*/
		QString GetUnitName() const;
		/**
		\brief Doubleclick handler
		\param evt Mouse event
		*/
		void mouseDoubleClickEvent ( QMouseEvent * evt );
		
		/**
		\brief Gets mode of OY axis maximum drawing
		\return mode
		*/
		MaxMode GetMaxMode() const;
		
	
	public slots:
		/**
		\brief Adds value to given dataset
		\param idx Dataset index
		\param val Value to add
		\param update Whether update widget after adding
		*/
		void AddValue(const size_t idx, const double val, bool update = true);
		/**
		\brief Adds dataset
		\param Cdd Dataset
		\param max Whether mark maximum of this dataset
		*/
		void AddValues(ChartDrawerData Cdd, const bool max = true);
		/**
		\brief Adds dataset
		\param Cdd Dataset
		\param idx Where
		\param max Whether mark maximum of this dataset
		*/
		void AddValues(ChartDrawerData Cdd, const size_t idx, const bool max = true);
		/**
		\brief Adds empty dataset
		\param rN Set's data name
		\param max Whether mark maximum of this dataset
		*/
		void AddValuesCnt(const QString & rN, const bool max = true);
		/**
		\brief Adds empty dataset
		\param rP Pen that will be used to drawing
		\param rN Dataset name
		\param max Whether mark maximum of this dataset
		*/
		void AddValuesCnt(const QPen & rP, const QString & rN, const bool max = true );
		
		/**
		\brief Removes dataset
		\param idx Dataset index
		*/
		void RemoveValuesCnt(const size_t idx);
		/**
		\brief Zeroes values
		\param idx Dataset index
		*/
		void Zero(const size_t idx);
		
		///Finds and sets maximum
		void FindSetMax();
		
		/**
		\brief Toggles marking of the maximum Y value on given dataset
		\param at dataset
		\param e Toggle?
		*/
		void EnableMaxDrawAt(const size_t, const bool);
		/**
		\brief Toggles automatic max Y scale settin
		\param a Toggle?
		*/
		void EnableAutoMax(bool a);
		
		/**
		\brief Sets maximum X
		\param x X size
		*/
		void SetXMax(const wgtsize_t x);
		/**
		\brief Sets maximum Y
		\param y Y size
		*/
		void SetYMax(const wgtsize_t x);
		
		/**
		\brief Sets mode of max of OY axis
		\param mm Mode
		*/
		void SetMaxMode(const MaxMode mm);
		
		
	signals:
		/**
		\brief Emited when widget is doubleclicked
		\param evt Mouse event
		*/
		void DoubleClicked(QMouseEvent * evt);
	
};

}

#endif

