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

#include "ChartDrawer.h"

#ifdef USE_SOLARIS
#include <ieeefp.h>
int isinf(double x) { return !finite(x) && x==x; }
#endif


namespace kt {

ChartDrawer::ChartDrawer(QWidget *p, wgtsize_t x_max, wgtsize_t y_max, bool autom, const QString & uname) : QWidget(p), mXMax(x_max), mYMax(y_max), mAutoMax(autom),
													mUnitName(uname), mMMode(MaxModeExact)
{
	setBackgroundColor("white");
}

ChartDrawer::~ChartDrawer()
{
	QToolTip::remove(this);
}

ChartDrawer::wgtsize_t ChartDrawer::GetXMax() const
{
	return mXMax;
}

ChartDrawer::wgtsize_t ChartDrawer::GetYMax() const
{
	return mYMax;
}

void ChartDrawer::SetXMax(const wgtsize_t x)
{
	mXMax = x;
	
	for(size_t i = 0; i < mEls.size(); i++)
 	{
 		mEls[i].pmVals -> resize(x, 0.0);
 	}
}

void ChartDrawer::SetYMax(const wgtsize_t y)
{
	mYMax = y;
}

inline ChartDrawer::wgtsize_t ChartDrawer::GetYScale() const
{
	return height() / 8;
}


inline ChartDrawer::wgtunit_t ChartDrawer::TrY(const ChartDrawer::wgtunit_t y) const
{
	return height() - y;
}

void ChartDrawer::paintEvent ( QPaintEvent *)
{
	QPainter pnt( this );

	DrawScale(pnt);
	DrawFrame(pnt);
	DrawChart(pnt);
	
}

inline ChartDrawer::wgtunit_t ChartDrawer::height() const
{
	return QWidget::height() - 15;
}

inline ChartDrawer::wgtunit_t ChartDrawer::width() const
{
	return QWidget::width() - 65;
}

void ChartDrawer::DrawFrame(QPainter & rPnt )
{
	QPen op = rPnt.pen();
	rPnt.setPen(QPen("#000", 3));
	
	rPnt.drawLine(0, TrY(0), width()+3, TrY(0));
	rPnt.drawLine(width()+1, TrY(0), width()+1, TrY(QWidget::height()));
	
	QFont oldf(rPnt.font());
	QFont newf(oldf);
	newf.setWeight(QFont::Bold);
	newf.setPointSize(10);
	newf.setUnderline(1);

	rPnt.setFont(newf);
	rPnt.drawText(width() + 30, TrY(-7), mUnitName);
	rPnt.setFont(oldf);
	
	rPnt.setPen(op);
}

void ChartDrawer::DrawScale(QPainter & rPnt )
{

	if(!mYMax)
	{
		return;
	}
	
	QPen op = rPnt.pen();
	QPen ep("#eee", 1, Qt::DashLine);
	QPen lp("#666", 2, Qt::DotLine);
	QPen tp("#000");
	
	rPnt.setPen(ep);
	
	for(wgtsize_t i = 1; i < width(); i += 10)
	{	
		rPnt.drawLine(i, TrY(0), i, TrY(height()));
	}
	
	for(wgtsize_t i = 0; i < height(); i += 10)
	{	
		rPnt.drawLine(0, TrY(i), width(), TrY(i));
	}
	
	rPnt.setPen(lp);
	rPnt.drawLine(0, TrY(height() - 10), width(), TrY(height() - 10));
	rPnt.setPen(tp);
	rPnt.drawText(width() + 4, TrY(height() - 10) + 4, QString::number (mYMax));
	
	for(wgtsize_t i = 0; i < height() - 15 ; i += GetYScale())
	{
		rPnt.setPen(lp);
		rPnt.drawLine(0, TrY(i), width(), TrY(i));
		rPnt.setPen(tp);
		rPnt.drawText(width() + 4, TrY(i) + 4, QString::number ( (mYMax / 8.0 ) * ( i / static_cast<double>(GetYScale() )), 'f', 1 ) );
	}
	
	rPnt.setPen(op);
}

void ChartDrawer::DrawChart(QPainter & rPnt)
{

	QPen op = rPnt.pen();
	
	uint32_t skip_max = 0;
	
 	for(size_t i = 0; i < mEls.size(); i++)
 	{
 		rPnt.setPen( *mEls[i].GetPen() );
 		
 		for(size_t j = 1; j < mEls[i].pmVals -> size() - 1; j++)
 		{
			rPnt.drawLine(
				FindXScreenCoords(j-1),
				TrY(FindYScreenCoords(mEls[i].pmVals -> at(j-1))),
				FindXScreenCoords(j),
				TrY(FindYScreenCoords(mEls[i].pmVals -> at(j)))
			);
 		}
// 	
		rPnt.drawLine(
			FindXScreenCoords(mEls[i].pmVals -> size() - 2),
			TrY(FindYScreenCoords(mEls[i].pmVals -> at(mEls[i].pmVals -> size() - 2))),
			width(),
			TrY(FindYScreenCoords(mEls[i].pmVals -> at(mEls[i].pmVals -> size() - 1)))
		);
		
		// --------------------
		//	Line on top
		// ------------
		QPen myop(rPnt.pen());
		QPen topl(myop);
		topl.setStyle(Qt::DotLine);	
		rPnt.setPen(topl);
		rPnt.drawLine(0, TrY(FindYScreenCoords(mEls[i].pmVals -> at(mEls[i].pmVals -> size() - 1))), width(), TrY(FindYScreenCoords(mEls[i].pmVals -> at(mEls[i].pmVals -> size() - 1))) );
		rPnt.setPen(myop);
		
		QFont oldf(rPnt.font());
		QFont newf(oldf);
		newf.setWeight(QFont::Bold);
		newf.setPointSize(8);

		rPnt.setFont(newf);
		rPnt.drawText(5 + (i * 50), TrY(FindYScreenCoords(mEls[i].pmVals -> at(mEls[i].pmVals -> size() - 1))) + 11, QString::number (mEls[i].pmVals -> at(mEls[i].pmVals -> size() - 1), 'f', 2 ) );

		
		//------------------
		//  max
		//------------------
		
		if(mMarkMax[i])
		{
			rPnt.setPen(topl);
			std::pair<double, size_t> max = mEls[i] . Max();
			
			rPnt.drawLine(
				FindXScreenCoords(max.second), TrY(0), FindXScreenCoords(max.second), TrY(height())
			);	
			
			rPnt.setPen(myop);
			
			rPnt.setFont(newf);
			QString maxv(QString::number (max.first, 'f', 2));
			
			if(FindXScreenCoords(max.second) < 35)
			{
				rPnt.drawText(FindXScreenCoords(max.second) + 5, TrY(height() - (10 * (i - skip_max)) ) + 10, maxv ) ;
			} else {
				rPnt.drawText(FindXScreenCoords(max.second) - 35 , TrY(height() - (10 * (i - skip_max)) ) + 10, maxv ) ;
			}
		} else {
			skip_max++;
		}
		
		rPnt.setFont(oldf);
		rPnt.setPen(op);
 	}
 	
 	rPnt.setPen(op);
}

inline ChartDrawer::wgtunit_t ChartDrawer::FindXScreenCoords(const double x) const
{
	return static_cast<wgtunit_t>((width() / static_cast<double>(mXMax)) * x) ;
}

inline ChartDrawer::wgtunit_t ChartDrawer::FindYScreenCoords(const double y) const
{
	return static_cast<wgtunit_t>(((height()) / static_cast<double>(mYMax)) * y) ;
}

void ChartDrawer::EnableAutoMax(bool a)
{
	mAutoMax = a;
}

void ChartDrawer::AddValue(const size_t idx, const double val, bool u )
{

	if( idx >= mEls.size()  )
	{
		return;
	}
	
	ChartDrawerData::val_t::iterator it = mEls[idx].pmVals -> begin();
	
	while(it != mEls[idx] .pmVals ->  end() )
	{
		*it = *(it + 1);
		it++;
	}
	
#ifdef USE_SOLARIS
	if(isnand(val) || (isinf(val)))
#else
 	if(std::isnan(val) || (std::isinf(val)))
#endif
	{
		*(mEls[idx].pmVals -> end() -1) = 0.0;
	} else {
		*(mEls[idx].pmVals -> end() -1) = val;
	}
	
	if(mAutoMax)
	{	
		if( (mMMode == MaxModeTop) && (val > mYMax) )
		{	
			mYMax = static_cast<wgtsize_t>(val) + 3;
	
		} else if(mMMode == MaxModeExact) {
			FindSetMax();
		}
	}
	
	if(u)
	{
		update();
	}

}

void ChartDrawer::AddValues(ChartDrawerData Cdd, const bool max)
{
	if(Cdd.pmVals -> size() != mXMax)
	{
		Cdd.pmVals -> resize(mXMax, 0.0);
	}
	
	mEls.push_back(Cdd);
	mMarkMax.push_back(max);
	
	MakeLegendTooltip();
}

void ChartDrawer::AddValues(ChartDrawerData Cdd, const size_t idx, const bool max)
{
	if(Cdd.pmVals -> size() != mXMax)
	{
		Cdd.pmVals -> resize(mXMax, 0.0);
	}

	if(idx >= mEls.size())
	{	
		mEls.push_back(Cdd);
	} else {
		mEls.insert(mEls.begin() + idx, Cdd);
	}
	
	if(idx >= mMarkMax.size())
	{
		mMarkMax.push_back(max);
	} else {
		mMarkMax.insert(mMarkMax.begin() + idx, max);
	}
	
	MakeLegendTooltip();
}

void ChartDrawer::AddValuesCnt(const QString & rN, const bool max)
{
	mEls.push_back(ChartDrawerData(mXMax, rN));
	mMarkMax.push_back(max);
	
	MakeLegendTooltip();
}

void ChartDrawer::AddValuesCnt(const QPen & rP, const QString & rN, const bool max)
{
	mEls.push_back(ChartDrawerData(rP, mXMax, rN));
	mMarkMax.push_back(max);
	
	MakeLegendTooltip();
}

void ChartDrawer::SetUnitName(const QString & rN)
{
	mUnitName = rN;
}

QString ChartDrawer::GetUnitName() const
{
	return mUnitName;
}

void ChartDrawer::mouseDoubleClickEvent ( QMouseEvent * evt )
{
	FindSetMax();
	
	emit DoubleClicked(evt);
}

void ChartDrawer::EnableMaxDrawAt(const size_t at, const bool e)
{
	if(at >= mMarkMax.size())
	{
		return;
	}
	
	mMarkMax[at] = e;
}

void ChartDrawer::RemoveValuesCnt(const size_t idx)
{
	if(idx >= mEls.size())
	{
		return;
	}
	
	mEls.erase(mEls.begin() + idx);
	
	if(idx <= mMarkMax.size())
	{
		mMarkMax.erase(mMarkMax.begin() + idx);
	}
	
	MakeLegendTooltip();
	
}

void ChartDrawer::Zero(const size_t idx)
{
	if(idx >= mEls.size())
	{
		return;
	}
	
	std::fill(mEls[idx].pmVals -> begin(), mEls[idx].pmVals -> end(), 0.0);
	
	if(mAutoMax)
	{
		mYMax = 1;
	}
}

void ChartDrawer::MakeLegendTooltip()
{
	QToolTip::remove(this);
	
	QString helpstr(QString("<b>%1:</b><br><br>").arg(i18n("Legend")));
	QMimeSourceFactory* factory = QMimeSourceFactory::defaultFactory();
	std::vector<QImage> img;
	
	for(size_t i = 0; i < mEls.size(); i++)
	{
		img.push_back(QImage(16,16, 32));
		img[i].fill(mEls[i].GetPen() -> color().pixel());
		
		for(uint8_t px = 0; px < 16; px++)
		{
			img[i].setPixel(px, 0, 0); //t
			img[i].setPixel(0, px, 0); //l
			img[i].setPixel(px, 15, 0); //b
			img[i].setPixel(15, px, 0); //r
		}
		
		factory->setImage(mEls[i].GetName().replace(' ', '_') + "-" + QString::number(i), img[i]);
		helpstr += QString("<img src='%1'>&nbsp;&nbsp;-&nbsp;&nbsp;%2<br>").arg(mEls[i].GetName().replace(" ", "_") + "-" + QString::number(i)).arg( mEls[i].GetName() );	
	}
	
	QToolTip::add(this, helpstr);
}

void ChartDrawer::FindSetMax()
{
	wgtsize_t mymax = 1;
	
	for(val_t::const_iterator it = mEls.begin(); it != mEls.end(); ++it)
	{
		for(ChartDrawerData::val_t::const_iterator subit = it -> pmVals -> begin(); subit !=  it -> pmVals -> end(); ++subit)
		{
			if ( (*subit) > mymax )
			{
				mymax = static_cast<wgtsize_t>(*subit) + 3;
			}
		}
	}
	
	mYMax = mymax;
}

void ChartDrawer::SetMaxMode(const MaxMode mm)
{
	mMMode = mm;
}

ChartDrawer::MaxMode ChartDrawer::GetMaxMode() const
{
	return mMMode;
}

} //NS end

#include "ChartDrawer.moc"
