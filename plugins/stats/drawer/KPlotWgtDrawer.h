/*
    SPDX-FileCopyrightText: 2007 Krzysztof Kundzicz <athantor@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KPlotWgtDrawer_H_
#define KPlotWgtDrawer_H_

#include <algorithm>
#include <cstdint>
#include <list>
#include <memory>
#include <utility>
#include <vector>

#include <QEvent>
#include <QHelpEvent>
#include <QImage>
#include <QList>
#include <QMenu>
#include <QPalette>
#include <QToolTip>
#include <QUuid>
#include <QWidget>

#include <KPlotAxis>
#include <KPlotObject>
#include <KPlotPoint>
#include <KPlotWidget>

#include <drawer/ChartDrawer.h>

namespace kt
{
/** \brief Drawer class based on KPlotWidget
\author Krzysztof Kundzicz <athantor@gmail.com>
*/

class KPlotWgtDrawer : public KPlotWidget, public ChartDrawer
{
    Q_OBJECT

public:
    /// Type used as buffer
    typedef std::list<std::pair<size_t, wgtunit_t>> buff_t;
    typedef QList<KPlotObject *> val_t;

private:
    /// Uuids of plotObjects
    std::vector<QUuid> pmUuids;
    /// Buffer where new values are stored between GUI updates
    buff_t pmBuff;
    /// Descriptions of plotObjects
    std::vector<QString> pmDescs;
    /// Context menu
    QMenu *pmCtxMenu;

    /// Makes a context menu for widget
    void MakeCtxMenu();
    /** \brief Converts ChartDrawerData to KPlotObject
    \param rC ChartDrawerData object to convert
    \return Converted object
    */
    KPlotObject *cdd2kpo(const ChartDrawerData &rC) const;
    /// Adds points to chart from buffer
    void AddPointsFromBuffer();
    /// Marks max
    void MarkMax();

public:
    /// Constructor
    KPlotWgtDrawer(QWidget *p);

    void paintEvent(QPaintEvent *pPevt) override;
    QUuid getUuid(const size_t idx) const override;

    bool event(QEvent *) override;

public Q_SLOTS:
    void addValue(const size_t idx, const wgtunit_t val, const bool upd = false) override;
    void addDataSet(ChartDrawerData Cdd) override;
    void insertDataSet(const size_t idx, ChartDrawerData Cdd) override;
    void removeDataSet(const size_t idx) override;
    void zero(const size_t idx) override;
    void zeroAll() override;
    void setUnitName(const QString &rN) override;
    void setPen(const size_t idx, const QPen &rP) override;
    void setXMax(const wgtunit_t x) override;
    void setYMax(const wgtunit_t y) override;
    void findSetMax() override;
    void setUuid(const size_t idx, const QUuid &rQ) override;
    int16_t findUuidInSet(const QUuid &rQ) const override;
    void setMaxMode(const MaxMode mm) override;
    void update() override;
    void setLegend(const QString &rL) override;
    QString makeLegendString() override;

    void enableAntiAlias(bool aa) override;
    void enableBackgroundGrid(bool bg) override;

    void showContextMenu(const QPoint &rP) override;
    void renderToImage() override;

Q_SIGNALS:
    void Zeroed(ChartDrawer *);
};

} // ns end

#endif
