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
#include <kfiledialog.h>
#include <kurl.h>
#include <kmenu.h>

#include <drawer/ChartDrawer.h>

namespace kt {

/** \brief Drawer class based on KPlotWidget
\author Krzysztof Kundzicz <athantor@gmail.com>
*/

class KPlotWgtDrawer : public KPlotWidget, public ChartDrawer
{
	Q_OBJECT
	
	public:
		///Type used as buffer
		typedef std::list<std::pair<size_t, wgtunit_t> > buff_t;
		typedef QList< KPlotObject * > val_t;
	
	private:
		///Uuids of plotObjects
		std::auto_ptr<std::vector<QUuid> > pmUuids;
		///Buffer where new values are stored between GUI updates
		std::auto_ptr<buff_t> pmBuff;
		///Descriptions of plotObjects
		std::auto_ptr<std::vector<QString> > pmDescs;
		///Context menu
		std::auto_ptr<KMenu> pmCtxMenu;
		
		///Makes a context menu for widget
		void MakeCtxMenu();
		/** \brief Converts ChartDrawerData to KPlotObject
		\param rC ChartDrawerData object to convert
		\return Converted object
		*/
		KPlotObject * cdd2kpo(const ChartDrawerData & rC) const;
		///Adds points to chart from buffer
		void AddPointsFromBuffer();
		///Marks max
		void MarkMax();
		
		
	public:
		///Constructor
		KPlotWgtDrawer(QWidget *p);
		
		void paintEvent ( QPaintEvent * pPevt );
		const QUuid * GetUuid(const size_t idx) const;
		
 		bool event( QEvent * ) ;
	
	public slots:
		void AddValue (const size_t idx, const wgtunit_t val, const bool upd=false);
		void AddDataSet (ChartDrawerData Cdd);
		void InsertDataSet (const size_t idx, ChartDrawerData Cdd);
		void RemoveDataSet (const size_t idx);
		void Zero (const size_t idx);
		void ZeroAll();
		void SetUnitName(const QString & rN);
		void SetPen (const size_t idx, const QPen &rP);
		void SetXMax (const wgtunit_t x);
		void SetYMax (const wgtunit_t y);
		void FindSetMax ();
		void SetUuid (const size_t idx, const QUuid &rQ);
		int16_t FindUuidInSet (const QUuid &rQ) const;
		void SetMaxMode (const MaxMode mm);
		void update ();
		void SetLegend(const QString & rL);
		QString MakeLegendStr();

		void EnableAntiAlias (const bool aa);
		void EnableBgdGrid(bool bg);
		
		void ShowCtxMenu(const QPoint & rP);
		void RenderToImage();
		
		
		
};

} // ns end

#endif
