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

#include "ChartDrawerData.h"

namespace kt {

ChartDrawerData::ChartDrawerData(const QString & rN) : pmQp(new QPen("#000", 1, Qt::SolidLine)), pmVals(new val_t(2, 0.0)), mName(rN)
{
}

ChartDrawerData::ChartDrawerData(const size_t s, const QString & rN) : pmQp(new QPen("#000", 1, Qt::SolidLine)), pmVals(new val_t(s, 0.0)), mName(rN)
{
}

ChartDrawerData::ChartDrawerData(const QPen & rQp, const QString & rN) : pmQp(new QPen(rQp)), pmVals(new val_t(2, 0.0)), mName(rN)
{
}

ChartDrawerData::ChartDrawerData(const QPen & rQp, const size_t s, const QString & rN) : pmQp(new QPen(rQp)), pmVals(new val_t(s, 0.0)), mName(rN)
{
}

ChartDrawerData::ChartDrawerData(const ChartDrawerData & rS)
{
	pmQp = new QPen(*rS.pmQp);
	pmVals = new val_t(*rS.pmVals);
	mName = rS.mName;
}

ChartDrawerData::~ChartDrawerData()
{
	delete pmQp;
	delete pmVals;
}

const ChartDrawerData::val_t * ChartDrawerData::GetVals() const
{
	return pmVals;
}

const QPen * ChartDrawerData::GetPen() const
{
	return pmQp;
}

void ChartDrawerData::SetPen(const QPen & rQp)
{
	delete pmQp;
	pmQp = new QPen(rQp);
}

std::pair<double, size_t> ChartDrawerData::Max() const 
{

	double max = 0.0;
	size_t cpos = 0;
	size_t maxpos = 0;

	for(val_t::iterator it = pmVals -> begin(); it != pmVals -> end(); it++)
	{
		if(max <= *it)
		{
			max = *it;
			maxpos = cpos;
		}
			
		cpos++;
	}
	
	return std::make_pair(max, maxpos);
}

QString ChartDrawerData::GetName() const
{
	return mName;
}
void ChartDrawerData::SetName( const QString & rN )
{
	mName = rN;
}

} // NS end
