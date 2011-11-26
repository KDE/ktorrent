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

#include <ChartDrawerData.h>

namespace kt {

ChartDrawerData::ChartDrawerData() : pmName(new QString(i18n("Unknown"))), pmPen(new QPen("#f00")), pmVals(new val_t), pmUuid(new QUuid(QUuid::createUuid())), mMax(true)
{
}

ChartDrawerData::~ChartDrawerData()
{
}

ChartDrawerData::ChartDrawerData(const ChartDrawerData & rCdd) : pmName(new QString(*(rCdd.pmName))),
								pmPen( new QPen(*(rCdd.pmPen))),
								pmVals(new val_t(*(rCdd.pmVals))),
								pmUuid(new QUuid(*(rCdd.pmUuid)) ),
								mMax ( rCdd.mMax )
{
	
}

ChartDrawerData::ChartDrawerData(const QString & rN, const QPen & rP, const bool sm, const QUuid & rU) : pmName ( new QString(rN) ), 
									pmPen ( new QPen(rP)), pmVals(new val_t), pmUuid(new QUuid(rU)), mMax(sm)
{
}

void ChartDrawerData::setSize(const size_t s)
{
	if(s != pmVals->size())
	{
		pmVals->resize(s, 0.0);
	}
}

void ChartDrawerData::zero()
{
	std::fill(pmVals->begin(), pmVals->end(), 0.0);
}

void ChartDrawerData::addValue(const qreal val)
{
	std::copy(pmVals->begin() + 1, pmVals->end(), pmVals->begin());
	*(pmVals->end() - 1) = val;
}

const ChartDrawerData::val_t * ChartDrawerData::getValues() const
{
	return pmVals.get();
}

const QPen * ChartDrawerData::getPen() const
{
	return pmPen.get();
}

void ChartDrawerData::setPen(const QPen & rQp)
{
	pmPen.reset(new QPen(rQp));
}

const QString * ChartDrawerData::getName() const
{
	return pmName.get();
}

void ChartDrawerData::setName(const QString & rNm)
{
	pmName.reset(new QString(rNm));
}

const QUuid * ChartDrawerData::getUuid() const
{
	return pmUuid.get();
}

void ChartDrawerData::setUuid(const QUuid & rU)
{
	pmUuid.reset(new QUuid(rU));
}

std::pair<qreal, size_t> ChartDrawerData::findMax() const
{
	if(!pmVals->size())
	{
		return std::make_pair(0,0);
	}
		
	qreal max = pmVals->at(0);
	size_t idx = 0;
	
	for(size_t i = 0; i < pmVals->size(); i++)
	{
		if(pmVals->at(i) >= max)
		{
			max = pmVals->at(i);
			idx = i;
		}
	}
	
	return std::make_pair(max, idx);
}

bool ChartDrawerData::getMarkMax() const
{
	return mMax;
}

void ChartDrawerData::enableMarkMax(const bool e)
{
	mMax = e;
}

} // ns end

