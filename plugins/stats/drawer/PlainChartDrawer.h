/*
    SPDX-FileCopyrightText: 2007 Krzysztof Kundzicz <athantor@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef PlainChartDrawer_H_
#define PlainChartDrawer_H_

#include <QBrush>
#include <QFont>
#include <QFrame>
#include <QImage>
#include <QMenu>
#include <QPainter>
#include <QPalette>
#include <QPen>
#include <QString>
#include <QStyleOption>
#include <QWidget>

#include <memory>
#include <utility>

#include <ChartDrawer.h>
#include <ChartDrawerData.h>

namespace kt
{
/** \brief Basic chart drawer widget
\author Krzysztof Kundzicz <athantor@gmail.com>
*/

class PlainChartDrawer : public QFrame, public ChartDrawer
{
    Q_OBJECT
private:
    /// Pointer to context menu
    QMenu *pmCtxMenu;

    /// Height of the chart area ( \b not widget height!)
    inline wgtunit_t height() const;
    /// Width of the chart area ( \b not widget width!)
    inline wgtunit_t width() const;

    /// Translates coords
    inline wgtunit_t TY(const wgtunit_t) const;

    /** \brief Translates chart X coord to screen coord
    \param xcc X chart coord
    \return Screen X coord
    */
    inline wgtunit_t FindXScreenCoords(const wgtunit_t xcc) const;
    /** \brief Translates chart Y coord to screen coord
    \param ycc Y chart coord
    \return Screen Y coord
    */
    inline wgtunit_t FindYScreenCoords(const wgtunit_t ycc) const;

    /// Makes a context menu for widget
    void MakeCtxMenu();

    /** \brief Draws chart's scale
    \param rPnt Painter object
    */
    void DrawScale(QPainter &rPnt);
    /** \brief Draws chart's frame
    \param rPnt Painter object
    */
    void DrawFrame(QPainter &rPnt);
    /** \brief Draws chart
    \param rPnt Painter object
    */
    void DrawChart(QPainter &rPnt);

    /** \brief Draws chart's lines
    \param rPnt Painter object
    \param rCdd Dataset to draw
    */
    void DrawChartLine(QPainter &rPnt, const ChartDrawerData &rCdd);
    /** \brief Draws current values of the sets
    \param rPnt Painter object
    \param rCdd Dataset to draw
    \param idx Set's index
    */
    void DrawCurrentValue(QPainter &rPnt, const ChartDrawerData &rCdd, size_t idx);
    /** \brief Marks maximum values
    \param rPnt Painter object
    \param rCdd Dataset to draw
    \param idx Set's index
    */
    void DrawMaximum(QPainter &rPnt, const ChartDrawerData &rCdd, size_t idx);

public:
    /** \brief Constructor
    \param p Parent
    */
    PlainChartDrawer(QWidget *p = 0);
    /// Destructor
    ~PlainChartDrawer() override;

    /** \brief Widget's paint event
    \param pPevt Event
    */
    void paintEvent(QPaintEvent *pPevt) override;

public Q_SLOTS:
    void showContextMenu(const QPoint &rP) override;
    void renderToImage() override;

    void addValue(const size_t idx, const wgtunit_t val, const bool upd = false) override;
    void addDataSet(ChartDrawerData Cdd) override;
    void insertDataSet(const size_t idx, ChartDrawerData Cdd) override;
    void removeDataSet(const size_t idx) override;
    void zero(const size_t idx) override;
    void zeroAll() override;
    void setUnitName(const QString &rN) override
    {
        pmUnitName = rN;
    }
    void setPen(const size_t idx, const QPen &rP) override;
    void setXMax(const wgtunit_t x) override;
    void setYMax(const wgtunit_t y) override;
    void findSetMax() override;
    void setUuid(const size_t idx, const QUuid &rQ) override;
    int16_t findUuidInSet(const QUuid &rQ) const override;
    void setMaxMode(const MaxMode mm) override;
    QUuid getUuid(const size_t idx) const override;
    QString makeLegendString() override;
    void setLegend(const QString &rL) override;
    void update() override;

    void enableAntiAlias(bool aa) override;
    void enableBackgroundGrid(bool bg) override;

Q_SIGNALS:
    void Zeroed(ChartDrawer *);
};

} // ns end

#endif
