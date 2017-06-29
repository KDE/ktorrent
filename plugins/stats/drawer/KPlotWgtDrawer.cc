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

#include <QFileDialog>
#include <KLocalizedString>

namespace kt
{

    KPlotWgtDrawer::KPlotWgtDrawer(QWidget* p) : KPlotWidget(p), ChartDrawer(), pmCtxMenu(new QMenu(this))
    {
        setLimits(0, mXMax, 0, mYMax);
        axis(TopAxis)->setVisible(false);
        axis(LeftAxis)->setVisible(false);

        axis(RightAxis)->setLabel(pmUnitName);
        axis(RightAxis)->setTickLabelsShown(true);

        setBackgroundColor(QPalette().color(QPalette::Active, QPalette::Base));
        setForegroundColor(QPalette().color(QPalette::Text));
        setGridColor(QPalette().color(QPalette::AlternateBase));

        setContextMenuPolicy(Qt::CustomContextMenu);
        MakeCtxMenu();
        connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));

    }

    void KPlotWgtDrawer::addValue(const size_t idx, const wgtunit_t val, const bool upd)
    {

        if (idx >= static_cast<size_t>(plotObjects().size()))
        {
            return;
        }

        pmBuff.push_back(std::make_pair(idx, val));

        if (upd)
        {
            update();
        }

    }

    void KPlotWgtDrawer::AddPointsFromBuffer()
    {

        if (!(pmBuff.size()))
        {
            return;
        }

        val_t kpo(plotObjects());

        while (pmBuff.size())
        {
            if ((pmBuff.front().first) >= static_cast<size_t>(kpo.size()))
            {
                pmBuff.pop_front();
                continue;
            }

            QList< KPlotPoint* > kpp = kpo[pmBuff.front().first]->points();

            if (kpp.size() > mXMax)
            {
                kpo[pmBuff.front().first]->removePoint(0);
            }

            for (size_t i = kpp.size() - 1 ; ((kpp.size()) && (i > 0)); i--)
            {
                kpp[i]->setX(kpp[i]->x() - 1);
            }

            kpo[pmBuff.front().first]->addPoint(mXMax, pmBuff.front().second);

            if (mCurrMaxMode == MM_Top)
            {
                if ((pmBuff.front().second > 1) && (pmBuff.front().second > mYMax))
                {
                    mYMax = pmBuff.front().second + 5;
                }
            }
            else if (mCurrMaxMode == MM_Exact)
            {
                findSetMax();
            }

            pmBuff.pop_front();
        }
    }

    KPlotObject* KPlotWgtDrawer::cdd2kpo(const ChartDrawerData& rC) const
    {
        KPlotObject* kpo = new KPlotObject(rC.getPen().color(), KPlotObject::Lines, 1, KPlotObject::NoPoints);
        kpo->setPen(rC.getPen());

        return kpo;
    }

    void KPlotWgtDrawer::addDataSet(ChartDrawerData Cdd)
    {
        addPlotObject(cdd2kpo(Cdd));

        pmUuids.push_back(Cdd.getUuid());
        pmDescs.push_back(Cdd.getName());
    }

    void KPlotWgtDrawer::insertDataSet(const size_t idx, ChartDrawerData Cdd)
    {
        if (idx >= static_cast<size_t>(plotObjects().size()))
        {
            return;
        }

        addPlotObject(cdd2kpo(Cdd));

        val_t kpol(plotObjects());

        val_t dest_l;
        val_t::iterator src = kpol.begin();

        while (src != kpol.end())
        {
            dest_l.push_back(new KPlotObject((*src)->pen().color(), static_cast<KPlotObject::PlotType>(static_cast<int>((*src)->plotTypes())), (*src)->size(), (*src)->pointStyle()));

            src++;
        }

        removeAllPlotObjects();

        addPlotObjects(dest_l);

        pmUuids.insert(pmUuids.begin() + idx, Cdd.getUuid());
        pmDescs.insert(pmDescs.begin() + idx, Cdd.getName());

        zeroAll();

    }

    void KPlotWgtDrawer::removeDataSet(const size_t idx)
    {
        val_t kpol(plotObjects());

        if (idx >= static_cast<size_t>(kpol.size()))
        {
            return;
        }

        kpol.erase(kpol.begin() + idx);

        val_t dest_l;
        val_t::iterator src = kpol.begin();

        while (src != kpol.end())
        {
            dest_l.push_back(new KPlotObject((*src)->pen().color(), static_cast<KPlotObject::PlotType>(static_cast<int>((*src)->plotTypes())), (*src)->size(), (*src)->pointStyle()));
            src++;
        }

        removeAllPlotObjects();

        addPlotObjects(dest_l);

        pmUuids.erase(pmUuids.begin() + idx);
        pmDescs.erase(pmDescs.begin() + idx);

        zeroAll();
    }

    void KPlotWgtDrawer::zero(const size_t idx)
    {
        val_t kpol(plotObjects());

        if (idx >= static_cast<size_t>(kpol.size()))
        {
            return;
        }

        std::list<buff_t::iterator> lb;

        //iteration through elements to-be-removed is EVIL

        for (buff_t::iterator it = pmBuff.begin();
                it != pmBuff.end();
                it++)
        {
            if (it->first == idx)
            {
                lb.push_back(it);
            }
        }

        //EVIL I SAY!
        for (std::list<buff_t::iterator>::iterator it = lb.begin();
                it != lb.end();
                it++)
        {
            pmBuff.erase(*it);
        }

        kpol[idx]->clearPoints();

        findSetMax();
    }

    void KPlotWgtDrawer::zeroAll()
    {
        for (size_t i = 0; i < static_cast<size_t>(plotObjects().size()) ; i++)
        {
            zero(i);
        }

        emit Zeroed(this);
    }

    void KPlotWgtDrawer::setUnitName(const QString& rN)
    {
        pmUnitName = rN;
        axis(RightAxis)->setLabel(rN);
    }

    void KPlotWgtDrawer::setPen(const size_t idx, const QPen& rP)
    {
        val_t kpol(plotObjects());

        if (idx >= static_cast<size_t>(plotObjects().size()))
        {
            return;
        }

        kpol[idx]->setLinePen(rP);

        kpol[idx]->setLabelPen(rP);
        kpol[idx]->setPen(rP);
    }

    void KPlotWgtDrawer::enableAntiAlias(const bool aa)
    {
        mAntiAlias = aa;
        setAntialiasing(mAntiAlias);
    }

    void KPlotWgtDrawer::setXMax(const wgtunit_t x)
    {
        mXMax = x;
        setLimits(0, mXMax, 0, mYMax);
    }

    void KPlotWgtDrawer::setYMax(const wgtunit_t y)
    {
        mYMax = y;
        setLimits(0, mXMax, 0, mYMax);
    }

    void KPlotWgtDrawer::findSetMax()
    {
        val_t kpol(plotObjects());
        QList< KPlotPoint* > kppl;

        wgtunit_t max = 0;

        for (size_t i = 0; i < static_cast<size_t>(kpol.size()) ; i++)
        {
            kppl = kpol[i]->points();

            for (size_t p = 0; p < static_cast<size_t>(kppl.size()) ; p++)
            {
                if ((kppl[p]->y()) > max)
                {
                    max = kppl[p]->y();
                }
            }

        }

        setYMax(max + 5);
    }

    void KPlotWgtDrawer::setUuid(const size_t idx, const QUuid& rQ)
    {
        if (idx >= static_cast<size_t>(plotObjects().size()))
        {
            return;
        }

        pmUuids.at(idx) = rQ;
    }

    int16_t KPlotWgtDrawer::findUuidInSet(const QUuid& rQ) const
    {
        std::vector<QUuid>::const_iterator it = std::find(pmUuids.begin(), pmUuids.end(), rQ);

        if (it == pmUuids.end())
        {
            return -1;
        }
        else
        {
            return it - pmUuids.begin();
        }
    }

    void KPlotWgtDrawer::update()
    {
        AddPointsFromBuffer();

        KPlotWidget::update();
    }

    void KPlotWgtDrawer::setLegend(const QString& rL)
    {
        setToolTip(rL);
    }

    void KPlotWgtDrawer::paintEvent(QPaintEvent* pPevt)
    {
        MarkMax();

        KPlotWidget::paintEvent(pPevt);
    }

    void KPlotWgtDrawer::setMaxMode(const MaxMode mm)
    {
        mCurrMaxMode = mm;
    }

    QString KPlotWgtDrawer::makeLegendString()
    {
        QString lgnd;
        val_t kpo(plotObjects());

        lgnd += i18n("<h1 align='center' style='font-size: large; text-decoration: underline'>Legend:</h1><ul type='square'>");

        for (size_t i = 0; i < static_cast<size_t>(kpo.size()); i++)
        {
            lgnd += i18n("<li><span style='background-color: %1; font-size: 14px; font-family: monospace'>&nbsp;&nbsp;</span>&nbsp;—&nbsp;%2</li>",
                         kpo[i]->linePen().color().name(),
                         pmDescs.at(i)
                        );
        }

        return lgnd + QStringLiteral("</ul>");
    }

    QUuid KPlotWgtDrawer::getUuid(const size_t idx) const
    {
        if (idx >= static_cast<size_t>(plotObjects().size()))
        {
            return 0;
        }

        return pmUuids.at(idx);
    }

    void KPlotWgtDrawer::enableBackgroundGrid(bool bg)
    {
        mBgdGrid = bg;
        setShowGrid(bg);
    }

    bool KPlotWgtDrawer::event(QEvent* e)
    {
        if ((e->type()) == QEvent::ToolTip)
        {
            QToolTip::showText(dynamic_cast<QHelpEvent*>(e)->globalPos(), makeLegendString(), this);
            return true;
        }
        else
        {
            return KPlotWidget::event(e);
        }
    }

    void KPlotWgtDrawer::MakeCtxMenu()
    {

        connect(pmCtxMenu->addAction(i18nc("@action:inmenu", "Save as image…")), SIGNAL(triggered(bool)), this, SLOT(renderToImage()));

        pmCtxMenu->addSeparator();

        connect(pmCtxMenu->addAction(i18nc("@action:inmenu Recalculate the 0Y axis and then redraw the chart", "Rescale")), SIGNAL(triggered(bool)), this, SLOT(findSetMax()));

        pmCtxMenu->addSeparator();

        QAction* rst = pmCtxMenu->addAction(i18nc("@action:inmenu", "Reset"));

        connect(rst, SIGNAL(triggered(bool)), this, SLOT(zeroAll()));
    }

    void KPlotWgtDrawer::showContextMenu(const QPoint& pos)
    {
        pmCtxMenu->exec(mapToGlobal(pos));
    }

    void KPlotWgtDrawer::renderToImage()
    {
        QString saveloc = QFileDialog::getSaveFileName(this, i18n("Select path to save image…"), i18n("Image files") + QLatin1String(" (*.png)"));

        if (!saveloc.length())
            return;

        QImage qi(width(), height(), QImage::Format_RGB32);
        render(&qi);
        qi.save(saveloc, "PNG", 0);
    }

    void KPlotWgtDrawer::MarkMax()
    {
        val_t kpol(plotObjects());
        QList< KPlotPoint* > kppl;

        wgtunit_t max;
        int32_t idx;

        for (size_t i = 0; i < static_cast<size_t>(kpol.size()); i++)
        {
            kppl = kpol[i]->points();
            max = 0;
            idx = -1;

            for (size_t j = 0; j < static_cast<size_t>(kppl.size()); j++)
            {
                if (kppl[j]->y() > max)
                {
                    max = kppl[j]->y();
                    idx = j;
                }

                kppl[j]->setLabel(QString());
            }

            if (idx >= 0)
            {
                kppl[idx]->setLabel(QString::number(max, 'f', 2));
            }

        }

    }

}// ns end
