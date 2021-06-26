/*
    SPDX-FileCopyrightText: 2007 Krzysztof Kundzicz <athantor@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <PlainChartDrawer.h>

#include <KLocalizedString>
#include <QFileDialog>

namespace kt
{

PlainChartDrawer::PlainChartDrawer(QWidget* p) :  QFrame(p), ChartDrawer(), pmCtxMenu(new QMenu(this))
{
    setStyleSheet(QStringLiteral(" background-color: ") % QPalette().color(QPalette::Active, QPalette::Base).name() % QLatin1Char(';'));

    setContextMenuPolicy(Qt::CustomContextMenu);
    MakeCtxMenu();

    connect(this, &PlainChartDrawer::customContextMenuRequested, this, &PlainChartDrawer::showContextMenu);
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

void PlainChartDrawer::paintEvent(QPaintEvent*)
{

    QStyleOption opt;
    opt.init(this);

    QPainter pnt(this);

    style()->drawPrimitive(QStyle::PE_Widget, &opt, &pnt, this);

    pnt.setRenderHint(QPainter::Antialiasing, mAntiAlias);
    pnt.setRenderHint(QPainter::TextAntialiasing, mAntiAlias);

    DrawScale(pnt);
    DrawFrame(pnt);
    DrawChart(pnt);
}

void PlainChartDrawer::DrawScale(QPainter& rPnt)
{
    if (!mYMax) {
        return;
    }


    QPen oldpen = rPnt.pen();

    QPen pen;

    if (mBgdGrid) {

        pen.setColor(QPalette().color(QPalette::AlternateBase));
        rPnt.setPen(pen);

        for (wgtunit_t i = 5; i < height(); i += 10) {
            rPnt.drawLine(0, TY(i), width(), TY(i));
        }

        for (wgtunit_t i = 5; i < width(); i += 10) {
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
    rPnt.drawText(width() + 4, TY(height() - 10) + 4, QString::number(mYMax, 'f', 1));

    for (wgtunit_t i = 0; i < height() - 15; i += scale) {
        rPnt.drawLine(0, TY(i), width(), TY(i));
        rPnt.drawText(width() + 5, TY(i) + 5, QString::number((mYMax / 8.0) * (i / scale), 'f', 1));
    }

    rPnt.setPen(oldpen);

    rPnt.setFont(oldfont);
}

void PlainChartDrawer::DrawFrame(QPainter& rPnt)
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

    rPnt.drawText(width() + 42., TY(-10.), pmUnitName);

    rPnt.setFont(oldf);
    rPnt.setPen(oldpen);

}

void PlainChartDrawer::DrawChart(QPainter& rPnt)
{
    QPen oldpen = rPnt.pen();

    for (size_t i = 0; i < pmVals.size(); i++) {
        DrawChartLine(rPnt, pmVals.at(i));
        DrawCurrentValue(rPnt, pmVals.at(i), i);

        if (pmVals.at(i).getMarkMax()) {
            DrawMaximum(rPnt, pmVals.at(i), i);
        }
    }

    rPnt.setPen(oldpen);
}

void PlainChartDrawer::DrawChartLine(QPainter& rPnt, const ChartDrawerData& rCdd)
{

    QPen qp = rCdd.getPen();
    qp.setJoinStyle(Qt::RoundJoin);
    rPnt.setPen(qp);

    const ChartDrawerData::val_t& vals = rCdd.getValues();

    QPointF* l = new QPointF[vals.size()];

    for (size_t i = 0; i < vals.size(); i++) {
        l[i] = QPointF(
                   FindXScreenCoords(i),
                   TY(FindYScreenCoords(vals.at(i)))
               );
    }

    l[vals.size() - 1] = QPointF(

                             width(),
                             TY(FindYScreenCoords(*(vals.end() - 1)))
                         );

    rPnt.drawPolyline(l, vals.size());
    delete [] l;
}

void PlainChartDrawer::DrawCurrentValue(QPainter& rPnt, const ChartDrawerData& rCdd, size_t idx)
{
    QPen qp = rCdd.getPen();
    qp.setJoinStyle(Qt::RoundJoin);

    QColor qc(qp.color());

    QFont oldfont(rPnt.font()), qf(oldfont);
    qf.setStretch(QFont::SemiCondensed);

    rPnt.setFont(qf);
    rPnt.setPen(qp);

    idx++;
    wgtunit_t y = -5 + (idx * 16);

    wgtunit_t val = *(rCdd.getValues().end() - 1);

    wgtunit_t lenmod;

    if (val <= 9.99) {
        lenmod = 19;
    } else if (val <= 99.99) {
        lenmod = 14;
    } else if (val <= 999.99) {
        lenmod = 7.5;
    } else if (val <= 9999.99) {
        lenmod = 1.5;
    } else {
        lenmod = -5;
    }


    rPnt.setBackgroundMode(Qt::OpaqueMode);

    rPnt.drawText(QWidget::width() - (40 - lenmod), y, QString::number(val, 'f', 2));
    rPnt.setBackgroundMode(Qt::TransparentMode);

    qc.setAlphaF(0.35);
    qp.setColor(qc);
    qp.setStyle(Qt::DashLine);
    rPnt.setPen(qp);

    QPointF l[3] = {
        QPointF(width(), TY(FindYScreenCoords(*(rCdd.getValues().end() - 1)))),
        QPointF(width() + (38 + lenmod), y + 2),
        QPointF(QWidget::width(), y + 2.5),
    };

    rPnt.drawPolyline(l, 3);

    rPnt.setFont(oldfont);
}

void PlainChartDrawer::DrawMaximum(QPainter& rPnt, const ChartDrawerData& rCdd, size_t idx)
{
    QPen qp = rCdd.getPen();
    QBrush oldb = qp.brush();
    QColor qc(qp.color());

    std::pair<qreal, size_t> max = rCdd.findMax();

    qc.setAlphaF(0.7);
    qp.setColor(qc);
    qp.setStyle(Qt::DashLine);
    rPnt.setPen(qp);
    rPnt.drawLine(FindXScreenCoords(max.second), TY(0), FindXScreenCoords(max.second), TY(height()));


    idx++;
    wgtunit_t y = 5.0 + (idx * 14);
    wgtunit_t x = FindXScreenCoords(max.second);

    if (x < 35) {
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

    rPnt.drawText(x, y, QString::number(max.first, 'f', 1));
    rPnt.setFont(oldfont);

    rPnt.setBackgroundMode(Qt::TransparentMode);
}

void PlainChartDrawer::MakeCtxMenu()
{

    connect(pmCtxMenu->addAction(i18nc("@action:inmenu", "Save as image…")),
    &QAction::triggered, this, [this](bool) {
        renderToImage();
    });

    pmCtxMenu->addSeparator();

    connect(pmCtxMenu->addAction(i18nc("@action:inmenu Recalculate the 0Y axis and then redraw the chart", "Rescale")),
    &QAction::triggered, this, [this](bool) {
        findSetMax();
    });

    pmCtxMenu->addSeparator();

    QAction* rst = pmCtxMenu->addAction(i18nc("@action:inmenu", "Reset"));

    connect(rst, &QAction::triggered, this, [this](bool) {
        zeroAll();
    });
}

void PlainChartDrawer::showContextMenu(const QPoint& pos)
{
    pmCtxMenu->exec(mapToGlobal(pos));
}

void PlainChartDrawer::renderToImage()
{
    QString saveloc = QFileDialog::getSaveFileName(this, i18n("Select path to save image…"), i18n("Image files") + QLatin1String(" (*.png)"));
    if (!saveloc.length())
        return;

    QImage qi(QWidget::width(), QWidget::height(), QImage::Format_RGB32);
    render(&qi);
    qi.save(saveloc, "PNG", 0);
}

void PlainChartDrawer::addValue(const size_t idx, const wgtunit_t val, const bool upd)
{
    if (idx >= pmVals.size()) {
        return;
    } else {
        pmVals[idx].addValue(val);
    }

    if (mCurrMaxMode == MM_Top) {
        if ((val > 1) && (val > mYMax)) {
            mYMax = val + 5;
        }
    } else if (mCurrMaxMode == MM_Exact) {
        findSetMax();
    }

    if (upd) {
        update();
    }

}

void PlainChartDrawer::addDataSet(ChartDrawerData Cdd)
{
    Cdd.setSize(mXMax);
    pmVals.push_back(Cdd);

    setLegend(makeLegendString());
}

void PlainChartDrawer::insertDataSet(const size_t idx, ChartDrawerData Cdd)
{
    pmVals.insert(pmVals.begin() + idx, Cdd);
    setLegend(makeLegendString());
}

void PlainChartDrawer::removeDataSet(const size_t idx)
{
    if (idx >= pmVals.size()) {
        return;
    } else {
        pmVals.erase((pmVals.begin()) + idx);
    }

    setLegend(makeLegendString());
}


void PlainChartDrawer::zero(const size_t idx)
{
    if (idx >= pmVals.size()) {
        return;
    } else {
        pmVals[idx].zero();
    }

    findSetMax();
}

void PlainChartDrawer::zeroAll()
{
    for (size_t idx = 0; idx < pmVals.size(); idx++) {
        pmVals[idx].zero();
    }

    findSetMax();

    Q_EMIT Zeroed(this);
}

void PlainChartDrawer::setMaxMode(const MaxMode mm)
{
    mCurrMaxMode = mm;
}

void PlainChartDrawer::setXMax(const wgtunit_t x)
{
    mXMax = x;

    for (size_t i = 0; i < pmVals.size(); i++) {
        pmVals.at(i).setSize(x);
    }
}


void PlainChartDrawer::setYMax(const wgtunit_t y)
{
    mYMax = y;
}

void PlainChartDrawer::setPen(const size_t idx, const QPen& rP)
{
    if (idx >= pmVals.size()) {
        return;
    }

    pmVals.at(idx).setPen(rP);

    makeLegendString();
}

QUuid PlainChartDrawer::getUuid(const size_t idx) const
{
    if (idx >= pmVals.size()) {
        return nullptr;
    }

    return pmVals.at(idx).getUuid();
}

void PlainChartDrawer::setUuid(const size_t idx, const QUuid& rU)
{
    if (idx >= pmVals.size()) {
        return;
    }

    pmVals.at(idx).setUuid(rU);
}

int16_t PlainChartDrawer::findUuidInSet(const QUuid& rU) const
{

    for (int16_t i = 0; i < static_cast<int16_t>(pmVals.size()); i++) {
        if (pmVals.at(i).getUuid() == rU) {
            return i;
        }
    }

    return -1;
}

void PlainChartDrawer::enableAntiAlias(bool aa)
{
    mAntiAlias = aa;
}

void PlainChartDrawer::findSetMax()
{
    wgtunit_t max = 1;

    for (size_t i = 0; i < pmVals.size(); i++) {
        wgtunit_t locval = pmVals.at(i).findMax().first;

        if (locval > max) {
            max = locval;
        }
    }

    mYMax = max + 5;
}

QString PlainChartDrawer::makeLegendString()
{
    QString lgnd = i18n("<h1 align='center' style='font-size: large; text-decoration: underline'>Legend:</h1><ul type='square'>");

    for (size_t i = 0; i < pmVals.size(); i++) {
        lgnd += i18n("<li><span style='background-color: %1; font-size: 14px; font-family: monospace'>&nbsp;&nbsp;</span>&nbsp;—&nbsp;%2</li>",
                     pmVals.at(i).getPen().color().name(),
                     pmVals.at(i).getName()
                    );
    }

    return lgnd + QStringLiteral("</ul>");
}

void PlainChartDrawer::setLegend(const QString& rL)
{
    setToolTip(rL);
}

void PlainChartDrawer::update()
{
    QFrame::update();
}

void PlainChartDrawer::enableBackgroundGrid(bool bg)
{
    mBgdGrid = bg;
}

} //NS end

