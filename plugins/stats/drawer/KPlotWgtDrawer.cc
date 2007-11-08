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

#include <KPlotWgtDrawer.h>

namespace kt {

KPlotWgtDrawer::KPlotWgtDrawer(QWidget *p) : KPlotWidget(p), ChartDrawer(), pmUuids(new std::vector<QUuid>), pmBuff(new buff_t), 
						pmDescs(new std::vector<QString>), pmCtxMenu(new KMenu(this))
{
	setLimits(0, mXMax, 0, mYMax);
	axis(TopAxis) -> setVisible(false);
	axis(LeftAxis) -> setVisible(false);
	
	axis(RightAxis) -> setLabel(*pmUnitName);
	axis(RightAxis) -> setTickLabelsShown(true);
	
	setBackgroundColor(QPalette().color(QPalette::Active, QPalette::Base));
	setForegroundColor(QPalette().color(QPalette::Text));
	setGridColor(QPalette().color(QPalette::AlternateBase));
	
	setContextMenuPolicy(Qt::CustomContextMenu);
	MakeCtxMenu();
	connect(this, SIGNAL(customContextMenuRequested ( const QPoint & )), this, SLOT(ShowCtxMenu ( const QPoint & )));
	
}

void KPlotWgtDrawer::AddValue (const size_t idx, const wgtunit_t val, const bool upd)
{

	if(idx >= static_cast<size_t>(plotObjects().size()) )
	{
		return;
	}
	
	pmBuff -> push_back(std::make_pair(idx, val));
	
	if(upd)
	{
		update();
	}
	
}

void KPlotWgtDrawer::AddPointsFromBuffer()
{

	if(!(pmBuff -> size()))
	{
		return;
	}
	
	val_t kpo(plotObjects());
	
	while(pmBuff -> size())
	{
		if( (pmBuff -> front().first) >= static_cast<size_t>(kpo.size()) )
		{
			pmBuff -> pop_front();
			continue;
		}
				
		QList< KPlotPoint * > kpp = kpo[pmBuff -> front().first] -> points();
	
		if(kpp.size() > mXMax)
		{
			kpo[pmBuff -> front().first] -> removePoint(0);
		} 
		
		for( size_t i = kpp.size() - 1 ; ( (kpp.size()) && (i > 0) ); i--)
		{
			kpp[i] -> setX(kpp[i] -> x() - 1);
		}	
			
		kpo[pmBuff -> front().first] -> addPoint( mXMax, pmBuff -> front().second);
		
		if(mCurrMaxMode == MM_Top)
		{
			if((pmBuff -> front().second > 1) && (pmBuff -> front().second > mYMax))
			{
				mYMax = pmBuff -> front().second + 5;
			}
		} else if (mCurrMaxMode == MM_Exact) {
			FindSetMax();
		}
		
		pmBuff -> pop_front();
	}
}

KPlotObject * KPlotWgtDrawer::cdd2kpo(const ChartDrawerData & rC) const
{
	KPlotObject * kpo = new KPlotObject(rC.GetPen() -> color(), KPlotObject::Lines, 1, KPlotObject::NoPoints);
	kpo -> setPen(*(rC.GetPen()));
	
	return kpo;
}

void KPlotWgtDrawer::AddDataSet (ChartDrawerData Cdd)
{
	addPlotObject(cdd2kpo(Cdd));
	
	pmUuids -> push_back(*(Cdd.GetUuid()));
	pmDescs -> push_back(*(Cdd.GetName()));
}

void KPlotWgtDrawer::InsertDataSet (const size_t idx, ChartDrawerData Cdd)
{
	if(idx >= static_cast<size_t>(plotObjects().size()) )
	{
		return;
	}
	
	addPlotObject(cdd2kpo(Cdd));	
	val_t kpol(plotObjects());
	
	val_t dest_l;
	val_t::iterator src = kpol.begin();
	
	while(src != kpol.end())
	{
		dest_l.push_back(new KPlotObject( (*src) -> pen().color(), static_cast<KPlotObject::PlotType>(static_cast<int>((*src) -> plotTypes())), (*src) -> size(), (*src) -> pointStyle()));
		
		src++;
	}
	
	removeAllPlotObjects();
	addPlotObjects(dest_l);
	
	pmUuids -> insert ( pmUuids -> begin() + idx, *(Cdd.GetUuid()));
	pmDescs -> insert ( pmDescs -> begin() + idx, *(Cdd.GetName()));
	
	ZeroAll();

}

void KPlotWgtDrawer::RemoveDataSet (const size_t idx)
{
	val_t kpol(plotObjects());

	if(idx >= static_cast<size_t>(kpol.size()) )
	{
		return;
	}
	
	kpol . erase (kpol.begin() + idx);
	
	val_t dest_l;
	val_t::iterator src = kpol.begin();
	
	while(src != kpol.end())
	{		
		dest_l.push_back(new KPlotObject( (*src) -> pen().color(), static_cast<KPlotObject::PlotType>(static_cast<int>((*src) -> plotTypes())), (*src) -> size(), (*src) -> pointStyle()));
		src++;
	}
	
	removeAllPlotObjects();
	addPlotObjects(dest_l);
	
	pmUuids -> erase(pmUuids -> begin() + idx);
	pmDescs -> erase(pmDescs -> begin() + idx);
	
	ZeroAll();
}

void KPlotWgtDrawer::Zero (const size_t idx)
{
	val_t kpol(plotObjects());

	if(idx >= static_cast<size_t>(kpol.size()) )
	{
		return;
	}
	
	kpol[idx] -> clearPoints();
}

void KPlotWgtDrawer::ZeroAll()
{
	for(size_t i = 0; i < static_cast<size_t>(plotObjects().size()) ; i++)
	{
		Zero(i);
	}
	
	pmBuff -> clear();
}

void KPlotWgtDrawer::SetUnitName(const QString & rN)
{
	*pmUnitName = rN;
	axis(RightAxis) -> setLabel(rN);
}

void KPlotWgtDrawer::SetPen (const size_t idx, const QPen &rP)
{
	val_t kpol(plotObjects());

	if(idx >= static_cast<size_t>(plotObjects().size()) )
	{
		return;
	}
	
	kpol[idx] -> setLinePen(rP);
	kpol[idx] -> setLabelPen(rP);
	kpol[idx] -> setPen(rP);
}

void KPlotWgtDrawer::EnableAntiAlias (const bool aa)
{
	mAntiAlias = aa;
	setAntialiasing(mAntiAlias);
}

void KPlotWgtDrawer::SetXMax (const wgtunit_t x)
{
	mXMax = x;
	setLimits(0, mXMax, 0, mYMax);
}

void KPlotWgtDrawer::SetYMax (const wgtunit_t y)
{
	mYMax = y;
	setLimits(0, mXMax, 0, mYMax);
}

void KPlotWgtDrawer::FindSetMax ()
{
	val_t kpol(plotObjects());
	QList< KPlotPoint * > kppl;
	
	wgtunit_t max = 0;
	
	for(size_t i = 0; i < static_cast<size_t>(kpol.size()) ; i++)
	{
		kppl = kpol[i] -> points();
		
		for(size_t p = 0; p < static_cast<size_t>(kppl.size()) ; p++)
		{
			if( (kppl[p] -> y()) > max )
			{
				max = kppl[p] -> y();
			}
		}
		
	}
	
	SetYMax(max + 5);
}

void KPlotWgtDrawer::SetUuid (const size_t idx, const QUuid &rQ)
{
	if(idx >= static_cast<size_t>(plotObjects().size()) )
	{
		return;
	}
	
	pmUuids -> at(idx) = rQ;
}

int16_t KPlotWgtDrawer::FindUuidInSet (const QUuid &rQ) const
{
	std::vector<QUuid>::iterator it = std::find(pmUuids -> begin(), pmUuids -> end(), rQ);
	 
	if(it == pmUuids -> end())
	{
		return -1;
	} else {
		return it - pmUuids -> begin();
	}
}

void KPlotWgtDrawer::update ()
{
	AddPointsFromBuffer();
	
	KPlotWidget::update();
}

void KPlotWgtDrawer::SetLegend(const QString & rL)
{
	setToolTip(rL);
}

void KPlotWgtDrawer::paintEvent ( QPaintEvent * pPevt )
{
	MarkMax();
	
	KPlotWidget::paintEvent(pPevt);
}

void KPlotWgtDrawer::SetMaxMode (const MaxMode mm)
{
	mCurrMaxMode = mm;
}

QString KPlotWgtDrawer::MakeLegendStr()
{
	QString lgnd("");
	val_t kpo(plotObjects());
	
	lgnd += i18n("<h1 align='center' style='font-size: large; text-decoration: underline'>Legend:</h1><ul type='square'>");
	
	for(size_t i = 0; i < static_cast<size_t>(kpo.size()); i++)
	{
		lgnd +=	i18n("<li><span style='background-color: %1; font-size: 14px; font-family: monospace'>&nbsp;&nbsp;</span>&nbsp;—&nbsp;%2</li>",
			kpo[i] -> linePen() . color().name(), 
			pmDescs -> at(i)
			);
	}
	
	return lgnd + "</ul>";
}

const QUuid * KPlotWgtDrawer::GetUuid(const size_t idx) const
{
	if(idx >= static_cast<size_t>(plotObjects().size()) )
	{
		return 0;
	}
	
	return &(pmUuids -> at(idx));
}

void KPlotWgtDrawer::EnableBgdGrid(bool bg)
{
	mBgdGrid = bg;
	setShowGrid(bg);
}

bool KPlotWgtDrawer::event( QEvent * e ) 
{
	if((e -> type()) == QEvent::ToolTip)
	{
		QToolTip::showText(dynamic_cast<QHelpEvent *>(e) -> globalPos(), MakeLegendStr(), this);
		return true;
	} else {
 		return KPlotWidget::event(e);
	}
}

void KPlotWgtDrawer::MakeCtxMenu()
{

	connect(pmCtxMenu -> addAction(i18n("Save as image…")), SIGNAL(triggered ( bool )), this, SLOT(RenderToImage()));
	
	pmCtxMenu -> addSeparator();
	
	connect(pmCtxMenu -> addAction(i18n("Rescale")), SIGNAL(triggered ( bool )), this, SLOT(FindSetMax()));
	
	pmCtxMenu -> addSeparator();
	
	QAction * rst = pmCtxMenu -> addAction(i18n("Reset"));
	
	connect(rst, SIGNAL(triggered ( bool )), this, SLOT(ZeroAll()));
	if(parent() != 0)
	{
		connect(rst, SIGNAL(triggered ( bool )), parent(), SLOT(ResetAvg()));
	}
}

void KPlotWgtDrawer::ShowCtxMenu(const QPoint & pos)
{
	pmCtxMenu -> exec( mapToGlobal(pos) );
}

void KPlotWgtDrawer::RenderToImage()
{
	QString saveloc = KFileDialog::	getSaveFileName (KUrl(), "image/png", this, i18n("Select path to save image…"));
	if(!saveloc.length())
	{
		return;
	}
	
	QImage qi(width(), height(), QImage::Format_RGB32);
	
	render(&qi);
	
	qi.save(saveloc, "PNG", 0);	
}

void KPlotWgtDrawer::MarkMax()
{
	val_t kpol(plotObjects());
	QList< KPlotPoint * > kppl;
	
	wgtunit_t max;
	int32_t idx;
	
	for(size_t i = 0; i < static_cast<size_t>(kpol.size()); i++)
	{
		kppl = kpol[i] -> points();
		max = 0;
		idx = -1;
		
		for(size_t j = 0; j < static_cast<size_t>(kppl.size()); j++)
		{
			if(kppl[j] -> y() > max)
			{
				max = kppl[j] -> y();
				idx = j;
			}
			
			kppl[j] -> setLabel(QString());
		}
		
		if(idx >= 0)
		{
			kppl[idx] -> setLabel(QString::number(max, 'f', 2));
		}
		
	}
	
}

}// ns end
