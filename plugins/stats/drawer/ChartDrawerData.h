/*
    SPDX-FileCopyrightText: 2007 Krzysztof Kundzicz <athantor@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ChartDrawerData_H_
#define ChartDrawerData_H_

#include <QPen>
#include <QPointer>
#include <QString>
#include <QUuid>

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

namespace kt
{
/** \brief „Container” for chart's data, used by ChartDrawer
\author Krzysztof Kundzicz <athantor@gmail.com>
*/

class ChartDrawerData
{
    friend class ChartDrawer;

public:
    /// Type storing chart's values
    typedef std::vector<qreal> val_t;

private:
    /// Name of the set
    QString pmName;
    /// Pent of the set
    QPen pmPen;
    /// Values
    val_t pmVals;
    /// Set's UUID
    QUuid pmUuid;
    /// Mark maximum?
    bool mMax;

public:
    /// Constructor
    ChartDrawerData();
    /// Destructor
    ~ChartDrawerData();
    /** \brief Copy constructor
    \param rCdd Source
    */
    ChartDrawerData(const ChartDrawerData &rCdd);
    /** \brief Constructor
    \param rN Name
    \param rP Pen
    \param max Mark maximum?
    \param rU UUID

    \note If there is no UUID given, it'll going be generated automagically
    */
    ChartDrawerData(const QString &rN, const QPen &rP, const bool max, const QUuid &rU = QUuid::createUuid());

    /** \brief Resizes set to given size
    \param size New size
    \note New values are filled with '0.0'
    */
    void setSize(const size_t size);

    /// Zeros the set
    void zero();

    /** \brief Adds value to set
    \param val Value
    */
    void addValue(const qreal val);

    /** \brief Returns valueset
    \return Values
    */
    const val_t &getValues() const
    {
        return pmVals;
    }

    /** \brief Returns set's pen
    \return Pen
    */
    QPen getPen() const
    {
        return pmPen;
    }
    /** \brief Sets set's pen
    \param rP New pen
    */
    void setPen(const QPen &rP)
    {
        pmPen = rP;
    }

    /** \brief Returns set's name
    \return Name
    */
    QString getName() const
    {
        return pmName;
    }

    /** \brief Sets set's name
    \param rN New name
    */
    void setName(const QString &rN)
    {
        pmName = rN;
    }

    /** \brief Returns set's UUID
    \return UUID
    */
    const QUuid getUuid() const
    {
        return pmUuid;
    }
    /** \brief Sets set's UUID
    \param rU New UUID
    */
    void setUuid(const QUuid &rU)
    {
        pmUuid = rU;
    }

    /** \brief Mark set's maximum
    \return Mark?
    */
    bool getMarkMax() const;
    /** \brief Enable maximum marking?
    \param max Enable?
    */
    void enableMarkMax(const bool max);

    /** \brief Finds maximum
     * \return Maximum
     *
     * Function returns pair, where:
     * - First: Maximum value
     * - Second: Value's position in set
     */
    std::pair<qreal, size_t> findMax() const;

    /// Dummy assignment operator for MSVC
    bool operator=(const ChartDrawerData & /*rCdd*/)
    {
        return true;
    };
};

} // NS end

#endif
