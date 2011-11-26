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

#include <PlainChartDrawer.h>

namespace kt {

PlainChartDrawer::PlainChartDrawer(QWidget * p) :  QFrame(p), ChartDrawer(), pmCtxMenu(new KMenu(this))
{
	setStyleSheet(" background-color: " + QPalette().color(QPalette::Active, QPalette::Base).name() + ";");
	
	setContextMenuPolicy(Qt::CustomContextMenu);
	MakeCtxMenu();
	
	connect(this, SIGNAL(customContextMenuRequested ( const QPoint & )), this, SLOT(ShowCtxMenu ( const QPoint & )));
}

PlainChartDrawer::~PlainChartDrawer()
{
}

inline PlainChartDrawer::wgtunit_t PlainChartDrawer::height() const
{
	return QWidget::height() - 15;
}

inline PlainChartDrawer::wgtunit_t PlainChartDrawer::width() const
{
	return QWidget::width() - 78;
}

inline PlainChartDrawer::wgtunit_t PlainChartDrawer::TY(const wgtunit_t y) const
{
	return height() - y;
}

inline PlainChartDrawer::wgtunit_t PlainChartDrawer::FindXScreenCoords(const wgtunit_t x) const
{
	return (width() / mXMax) * x ;
}

inline PlainChartDrawer::wgtunit_t PlainChartDrawer::FindYScreenCoords(const wgtunit_t y) const
{
	return (height() / mYMax) * y;
}

void PlainChartDrawer::paintEvent ( QPaintEvent * )
{
	
	QStyleOption opt;
	opt.init(this);
	
	QPainter pnt( this );
	
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &pnt, this);
	
	pnt.setRenderHint(QPainter::Antialiasing, mAntiAlias);
	pnt.setRenderHint(QPainter::TextAntialiasing, mAntiAlias);
		
	DrawScale(pnt);
	DrawFrame(pnt);
	DrawChart(pnt);
}

void PlainChartDrawer::DrawScale(QPainter & rPnt)
{
	if(!mYMax)
	{
		return;
	}
		
		
	QPen oldpen = rPnt.pen();
	QPen pen;
	
	if(mBgdGrid)
	{
	
		pen.setColor(QPalette().color(QPalette::AlternateBase));
		rPnt.setPen(pen);
		
		for(wgtunit_t i = 5; i < height(); i += 10)
		{
			rPnt.drawLine(0, TY(i), width(), TY(i));
		}
		
		for(wgtunit_t i = 5; i < width(); i += 10)
		{
			rPnt.drawLine(i, TY(0), i, TY(height()));
		}
	}
	
	wgtunit_t scale =  height() / 8;
	
	pen.setColor(QPalette().color(QPalette::Text));
	pen.setWidth(1);
	pen.setStyle(Qt::DotLine);
	
	rPnt.setPen(pen);
	
	QFont oldfont(rPnt.font()), qf(oldfont);
	qf.setStretch(QFont::SemiCondensed);
	rPnt.setFont(qf);
	
	rPnt.drawLine(0, TY(height() - 10), width(), TY(height() - 10));
	rPnt.drawText(width() + 4, TY(height() - 10) + 4, QString::number (mYMax, 'f', 1));
	
	for(wgtunit_t i = 0; i < height() - 15; i += scale)
	{
		rPnt.drawLine(0, TY(i), width(), TY(i));
		rPnt.drawText(width() + 5, TY(i) + 5, QString::number ( (mYMax / 8.0 ) * ( i / scale ), 'f', 1 ) );	
	}
	
	rPnt.setPen(oldpen);
	rPnt.setFont(oldfont);
}

void PlainChartDrawer::DrawFrame(QPainter & rPnt)
{
	QPen oldpen = rPnt.pen();
	QPen pen;
	
	pen.setColor(QPalette().color(QPalette::Text));
	pen.setWidth(3);
	rPnt.setPen(pen);
	
	QPoint points[3] = {
		QPoint(0, TY(0)),
		QPoint(width(), TY(0)),
		QPoint(width(), TY(height()))
	};
	
	rPnt.drawPolyline(points, 3);
	
	QFont oldf(rPnt.font());
	QFont newf(oldf);
	newf.setWeight(QFont::Bold);
	newf.setStretch(QFont::SemiCondensed);
	newf.setPointSize(10);
	newf.setUnderline(1);
	rPnt.setFont(newf);
	
	
	QColor qc(pen.color());
	qc.setAlphaF(0.75);
	pen.setColor(qc);
	rPnt.setPen(pen);
	
	rPnt.drawText(width() + 42., TY(-10.), *pmUnitName);
	
	rPnt.setFont(oldf);
	rPnt.setPen(oldpen);
	
}

void PlainChartDrawer::DrawChart(QPainter & rPnt)
{
	QPen oldpen = rPnt.pen();
	
	for(size_t i = 0; i < pmVals->size(); i++)
	{
		DrawChartLine(rPnt, pmVals->at(i));
		DrawCurrentValue(rPnt, pmVals->at(i), i);
		
		if(pmVals->at(i).GetMarkMax())
		{
			DrawMaximum(rPnt, pmVals->at(i), i);
		}
	}
	
	rPnt.setPen(oldpen);
}

void PlainChartDrawer::DrawChartLine(QPainter & rPnt, const ChartDrawerData & rCdd)
{

	QPen qp(*rCdd . GetPen());
	qp.setJoinStyle(Qt::RoundJoin);
	rPnt.setPen(qp);
	
	ChartDrawerData::val_t const * vals = rCdd . GetValues();
	
	QPointF * l = new QPointF[vals->size()];
	
	for(size_t i = 0; i < vals->size(); i++ )
	{
		l[i] = QPointF(
			FindXScreenCoords(i),
			TY(FindYScreenCoords(vals->at(i)))
		);
	}
	
	l[vals->size() -1] = QPointF(
			width(),
			TY(FindYScreenCoords( *(vals->end() - 1) ))
		);
	
	rPnt.drawPolyline(l, vals->size());
	delete [] l;
}

void PlainChartDrawer::DrawCurrentValue(QPainter & rPnt, const ChartDrawerData & rCdd, size_t idx)
{
	QPen qp(*rCdd . GetPen());
	qp.setJoinStyle(Qt::RoundJoin);
	
	QColor qc(qp.color());
	
	QFont oldfont(rPnt.font()), qf(oldfont);
	qf.setStretch(QFont::SemiCondensed);
	
	rPnt.setFont(qf);
	rPnt.setPen(qp);
	
	idx++;
	wgtunit_t y = -5 + (idx * 16);
	
	wgtunit_t val = *(rCdd . GetValues()->end() - 1);
	
	wgtunit_t lenmod;
	
	if( val <= 9.99 )
	{
		lenmod = 19;
	} 
	else if(val <= 99.99)
	{
		lenmod = 14;
	} else if(val <= 999.99)
	{
		lenmod = 7.5;
	} else if(val <= 9999.99)
	{
		lenmod = 1.5;
	} else {
		lenmod = -5;
	}
	
	
	rPnt.setBackgroundMode(Qt::OpaqueMode);
	rPnt.drawText(QWidget::width() - (40 - lenmod) , y, QString::number(val, 'f', 2) );
	rPnt.setBackgroundMode(Qt::TransparentMode);
	
	qc.setAlphaF(0.35);
	qp.setColor(qc);
	qp.setStyle(Qt::DashLine);
	rPnt.setPen(qp);
	
	QPointF l[3] = {
		QPointF(width(), TY(FindYScreenCoords( *(rCdd . GetValues()->end() - 1) ))),
		QPointF(width() + (38 + lenmod), y + 2),
		QPointF( QWidget::width() , y + 2.5),
	};
	
	rPnt.drawPolyline(l, 3);
	
	rPnt.setFont(oldfont);
}

void PlainChartDrawer::DrawMaximum(QPainter & rPnt, const ChartDrawerData & rCdd, size_t idx)
{
	QPen qp(*rCdd . GetPen());
	QBrush oldb = qp.brush();
	QColor qc(qp.color());
	
	std::pair<qreal, size_t> max = rCdd . FindMax();
	
	qc.setAlphaF(0.7);
	qp.setColor(qc);
	qp.setStyle(Qt::DashLine);
	rPnt.setPen(qp);
	rPnt.drawLine(FindXScreenCoords(max.second), TY(0),FindXScreenCoords(max.second), TY(height()));
	
		
	idx++;
	wgtunit_t y = 5.0 + (idx * 14);
	wgtunit_t x = FindXScreenCoords(max.second);
	
	if(x < 35)
	{
		x += 5;
	} else {
		x -= 35;
	}
		
	
	qc.setAlphaF(1);
	qp.setColor(qc);
	rPnt.setPen(qp);
	qp.setStyle(Qt::SolidLine);
	rPnt.setBackgroundMode(Qt::OpaqueMode);
	
	QFont oldfont(rPnt.font()), qf(oldfont);
	qf.setStretch(QFont::SemiCondensed);
	rPnt.setFont(qf);
	
	rPnt.drawText(x, y, QString::number(max.first, 'f', 1) );
	rPnt.setFont(oldfont);
	
	rPnt.setBackgroundMode(Qt::TransparentMode);
}

void PlainChartDrawer::MakeCtxMenu()
{

	connect(pmCtxMenu->addAction(i18n("Save as image…")), SIGNAL(triggered ( bool )), this, SLOT(RenderToImage()));
	
	pmCtxMenu->addSeparator();
	
	connect(pmCtxMenu->addAction(i18n("Rescale")), SIGNAL(triggered ( bool )), this, SLOT(FindSetMax()));
	
	pmCtxMenu->addSeparator();
	
	QAction * rst = pmCtxMenu->addAction(i18n("Reset"));
	
	connect(rst, SIGNAL(triggered ( bool )), this, SLOT(ZeroAll()));
}

void PlainChartDrawer::ShowCtxMenu(const QPoint & pos)
{
	pmCtxMenu->exec( mapToGlobal(pos) );
}

void PlainChartDrawer::RenderToImage()
{
	QString saveloc = KFileDialog::	getSaveFileName (KUrl("kfiledialog:///openTorrent"), "image/png", this, i18n("Select path to save image…"));
	if(!saveloc.length())
	{
		return;
	}
	
	QImage qi(QWidget::width(), QWidget::height(), QImage::Format_RGB32);

	render(&qi);
	
	qi.save(saveloc, "PNG", 0);
	
}

void PlainChartDrawer::AddValue (const size_t idx, const wgtunit_t val, const bool upd )
{
	if(idx >= pmVals->size())
	{
		return;
	} else {
		(*pmVals)[idx].AddValue(val);
	}
	
	if(mCurrMaxMode == MM_Top)
	{
		if((val > 1) && (val > mYMax))
		{
			mYMax = val + 5;
		}
	} else if (mCurrMaxMode == MM_Exact) {
		FindSetMax();
	}
	
	if(upd)
	{
		update();
	}

}

void PlainChartDrawer::AddDataSet (ChartDrawerData Cdd)
{
	Cdd . SetSize(mXMax);
	pmVals->push_back(Cdd);
	
	SetLegend(MakeLegendStr());
}

void PlainChartDrawer::InsertDataSet (const size_t idx, ChartDrawerData Cdd)
{
	pmVals->insert(pmVals->begin()+idx, Cdd);
	SetLegend(MakeLegendStr());
}

void PlainChartDrawer::RemoveDataSet (const size_t idx)
{
	if(idx >= pmVals->size())
	{
		return;
	} else {
		pmVals->erase( (pmVals->begin()) + idx);
	}
	
	SetLegend(MakeLegendStr());
}


void PlainChartDrawer::Zero (const size_t idx)
{
	if(idx >= pmVals->size())
	{
		return;
	} else {
		(*pmVals)[idx].Zero();
	}
	
	FindSetMax();
}

void PlainChartDrawer::ZeroAll ()
{
	for( size_t idx = 0; idx < pmVals->size(); idx++)
	{
		(*pmVals)[idx].Zero();
	} 
	
	FindSetMax();
	emit Zeroed(this);
}

void PlainChartDrawer::SetMaxMode (const MaxMode mm)
{
	mCurrMaxMode = mm;
}

void PlainChartDrawer::SetXMax (const wgtunit_t x)
{
	mXMax = x;
	for(size_t i = 0; i < pmVals->size(); i++)
	{
		pmVals->at(i).SetSize(x);
	}
}


void PlainChartDrawer::SetYMax (const wgtunit_t y)
{
	mYMax = y;
}

void PlainChartDrawer::SetUnitName(const QString & rUn)
{
	pmUnitName.reset(new QString(rUn));
}


void PlainChartDrawer::SetPen(const size_t idx, const QPen & rP)
{
	if(idx >= pmVals->size())
	{
		return;
	}
	
	pmVals->at(idx).SetPen(rP);
	
	MakeLegendStr();
}

const QUuid * PlainChartDrawer::GetUuid(const size_t idx) const
{
	if(idx >= pmVals->size())
	{
		return 0;
	}
	
	return pmVals->at(idx) . GetUuid();
}

void PlainChartDrawer::SetUuid(const size_t idx, const QUuid & rU)
{
	if(idx >= pmVals->size())
	{
		return;
	}
	
	pmVals->at(idx) . SetUuid(rU);
}

int16_t PlainChartDrawer::FindUuidInSet(const QUuid & rU) const
{
	
	for(int16_t i = 0; i < static_cast<int16_t>(pmVals->size()); i++)
	{
		if( (*(pmVals->at(i).GetUuid())) == rU)
		{
			return i;
		}
	}
	
	return -1;
}

void PlainChartDrawer::EnableAntiAlias(bool aa)
{
	mAntiAlias = aa;
}

void PlainChartDrawer::FindSetMax()
{
	wgtunit_t max = 1;
	
	for(size_t i = 0; i < pmVals->size(); i++)
	{
		wgtunit_t locval = pmVals->at(i).FindMax().first;
		if(locval > max)
		{
			max = locval;
		}
	}
		
	mYMax = max + 5;
}

QString PlainChartDrawer::MakeLegendStr()
{
	QString lgnd("");
	
	lgnd += i18n("<h1 align='center' style='font-size: large; text-decoration: underline'>Legend:</h1><ul type='square'>");
	
	for(size_t i = 0; i < pmVals->size(); i++)
	{
		lgnd +=	i18n("<li><span style='background-color: %1; font-size: 14px; font-family: monospace'>&nbsp;&nbsp;</span>&nbsp;—&nbsp;%2</li>",
			pmVals->at(i) . GetPen()->color().name(), 
			*(pmVals->at(i) . GetName())
			);
	}
	
	return lgnd + "</ul>";
}

void PlainChartDrawer::SetLegend(const QString & rL)
{
	setToolTip(rL);
}

void PlainChartDrawer::update ()
{
	QFrame::update();
}

void PlainChartDrawer::EnableBgdGrid(bool bg) 
{
	mBgdGrid = bg;
}

} //NS end

