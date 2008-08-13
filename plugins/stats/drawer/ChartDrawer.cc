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

#include <ChartDrawer.h>

namespace kt {

ChartDrawer::ChartDrawer () :		pmVals(new val_t), 
					pmUnitName(new QString(i18n("KB/s") ) ), 
					mCurrMaxMode(MM_Exact), 
					mXMax(16), 
					mYMax(1),
					mAntiAlias(1)
{
}

ChartDrawer::~ChartDrawer()
{
}

ChartDrawer::wgtunit_t ChartDrawer::GetXMax() const
{
	return mXMax;
}

ChartDrawer::wgtunit_t ChartDrawer::GetYMax() const
{
	return mYMax;
}


const QString * ChartDrawer::GetUnitName() const
{
	return pmUnitName.get();
}

size_t ChartDrawer::SetsCount() const
{
	return pmVals -> size();
}

ChartDrawer::val_t::const_iterator ChartDrawer::const_begin() const
{
	return pmVals -> begin();
}

ChartDrawer::val_t::const_iterator ChartDrawer::const_end() const
{
	return pmVals -> end();
}

} //NS end
