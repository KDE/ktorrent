/*
    SPDX-FileCopyrightText: 2007 Krzysztof Kundzicz <athantor@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <ChartDrawerData.h>

#include <KLocalizedString>

namespace kt
{

ChartDrawerData::ChartDrawerData() : pmName(i18n("Unknown")), pmPen("#f00"), pmUuid(QUuid::createUuid()), mMax(true)
{
}

ChartDrawerData::~ChartDrawerData()
{
}

ChartDrawerData::ChartDrawerData(const ChartDrawerData& rCdd) : pmName(rCdd.pmName),
    pmPen(rCdd.pmPen),
    pmVals(rCdd.pmVals),
    pmUuid(rCdd.pmUuid),
    mMax(rCdd.mMax)
{

}

ChartDrawerData::ChartDrawerData(const QString& rN, const QPen& rP, const bool sm, const QUuid& rU) : pmName(rN),
    pmPen(rP), pmUuid(rU), mMax(sm)
{
}

void ChartDrawerData::setSize(const size_t s)
{
    if (s != pmVals.size()) {
        pmVals.resize(s, 0.0);
    }
}

void ChartDrawerData::zero()
{
    std::fill(pmVals.begin(), pmVals.end(), 0.0);
}

void ChartDrawerData::addValue(const qreal val)
{
    std::copy(pmVals.begin() + 1, pmVals.end(), pmVals.begin());
    *(pmVals.end() - 1) = val;
}

std::pair<qreal, size_t> ChartDrawerData::findMax() const
{
    if (!pmVals.size()) {
        return std::make_pair(0, 0);
    }

    qreal max = pmVals.at(0);
    size_t idx = 0;

    for (size_t i = 0; i < pmVals.size(); i++) {
        if (pmVals.at(i) >= max) {
            max = pmVals.at(i);
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

