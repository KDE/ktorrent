/*
    SPDX-FileCopyrightText: 2007 Krzysztof Kundzicz <athantor@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <ChartDrawer.h>

#include <KLocalizedString>

namespace kt
{

ChartDrawer::ChartDrawer()
    : pmUnitName(i18n("KiB/s"))
    , mCurrMaxMode(MM_Exact)
    , mXMax(16)
    , mYMax(1)
    , mAntiAlias(1)
{
}

ChartDrawer::~ChartDrawer()
{
}

ChartDrawer::wgtunit_t ChartDrawer::getXMax() const
{
    return mXMax;
}

ChartDrawer::wgtunit_t ChartDrawer::getYMax() const
{
    return mYMax;
}

size_t ChartDrawer::dataSetCount() const
{
    return pmVals.size();
}

ChartDrawer::val_t::const_iterator ChartDrawer::begin() const
{
    return pmVals.begin();
}

ChartDrawer::val_t::const_iterator ChartDrawer::end() const
{
    return pmVals.end();
}

} //NS end
